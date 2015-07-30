/*
 * x86 MSR batch access device
 *
 * This device is accessed by ioctl() to submit a batch of MSR requests
 * which may be used instead of or in addition to the lseek()/write()/read()
 * mechanism provided by msr_safe.c
 *
 * This driver uses /dev/cpu/msr_batch as its device file.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/cpu.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <asm/msr.h>
#include "msr_whitelist.h"
#include "msr_batch.h"

static int majordev;
static struct class *cdev_class;
static char cdev_created;
static char cdev_registered;
static char cdev_class_created;

struct msrbatch_session_info {
	int rawio_allowed;
};

static int msrbatch_open(struct inode *inode, struct file *file)
{
	unsigned int cpu;
	struct cpuinfo_x86 *c;
	struct msrbatch_session_info *myinfo;

	cpu = iminor(file->f_path.dentry->d_inode);
	if (cpu >= nr_cpu_ids || !cpu_online(cpu)) {
		pr_err("cpu #%u does not exist\n", cpu);
		return -ENXIO; /* No such CPU */
	}

	c = &cpu_data(cpu);
	if (!cpu_has(c, X86_FEATURE_MSR)) {
		pr_err("cpu #%u does not have MSR feature.\n", cpu);
		return -EIO;	/* MSR not supported */
	}

	myinfo = kmalloc(sizeof(*myinfo), GFP_KERNEL);
	if (!myinfo)
		return -ENOMEM;

	myinfo->rawio_allowed = capable(CAP_SYS_RAWIO);
	file->private_data = myinfo;

	return 0;
}

static int msrbatch_close(struct inode *inode, struct file *file)
{
	kfree(file->private_data);
	file->private_data = 0;
	return 0;
}

static int msrbatch_apply_whitelist(struct msr_batch_array *oa,
				struct msrbatch_session_info *myinfo)
{
	struct msr_batch_op *op;
	int err = 0;

	for (op = oa->ops; op < oa->ops + oa->numops; ++op) {
		op->err = 0;
		if (myinfo->rawio_allowed) {
			op->wmask = 0xffffffffffffffff;
			continue;
		}

		if (!msr_whitelist_maskexists(op->msr)) {
			pr_err("No whitelist entry for MSR %x\n", op->msr);
			op->err = err = -EACCES;
		} else {
			op->wmask = msr_whitelist_writemask(op->msr);
			/*
			 * Check for read-only case
			 */
			if (op->wmask == 0 && !op->isrdmsr) {
				if (!myinfo->rawio_allowed) {
					pr_err("MSR %x is read-only\n",
								op->msr);
					op->err = err = -EACCES;
				}
			}
		}
	}
	return err;
}

static long msrbatch_ioctl(struct file *f, unsigned int ioc, unsigned long arg)
{
	int err = 0;
	struct msr_batch_array __user *uoa;
	struct msr_batch_op __user *uops;
	struct msr_batch_array koa;
	struct msrbatch_session_info *myinfo = f->private_data;

	if (ioc != X86_IOC_MSR_BATCH) {
		pr_err("Invalid ioctl op %u\n", ioc);
		return -ENOTTY;
	}

	if (!(f->f_mode & FMODE_READ)) {
		pr_err("File not open for reading\n");
		return -EBADF;
	}

	uoa = (struct msr_batch_array *)arg;

	if (copy_from_user(&koa, uoa, sizeof(koa))) {
		pr_err("Copy of batch array descriptor failed\n");
		return -EFAULT;
	}

	if (koa.numops <= 0) {
		pr_err("Invalid # of ops %d\n", koa.numops);
		return -EINVAL;
	}

	uops = koa.ops;

	koa.ops = kmalloc_array(koa.numops, sizeof(*koa.ops), GFP_KERNEL);
	if (!koa.ops)
		return -ENOMEM;

	if (copy_from_user(koa.ops, uops, koa.numops * sizeof(*koa.ops))) {
		pr_err("Copy of batch array failed\n");
		err = -EFAULT;
		goto bundle_alloc;
	}

	err = msrbatch_apply_whitelist(&koa, myinfo);
	if (err) {
		pr_err("Failed to apply whitelist %d\n", err);
		goto copyout_and_return;
	}

	err = msr_safe_batch(&koa);
	if (err != 0) {
		pr_err("msr_safe_batch failed: %d\n", err);
		goto copyout_and_return;
	}

copyout_and_return:
	if (copy_to_user(uops, koa.ops, koa.numops * sizeof(*uops))) {
		pr_err("copy batch data back to user failed\n");
		if (!err)
			err = -EFAULT;
	}
bundle_alloc:
	kfree(koa.ops);

	return err;
}

static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = msrbatch_open,
	.unlocked_ioctl = msrbatch_ioctl,
	.compat_ioctl = msrbatch_ioctl,
	.release = msrbatch_close
};

void msrbatch_cleanup(void)
{
	if (cdev_created) {
		cdev_created = 0;
		device_destroy(cdev_class, MKDEV(majordev, 0));
	}

	if (cdev_class_created) {
		cdev_class_created = 0;
		class_destroy(cdev_class);
	}

	if (cdev_registered) {
		cdev_registered = 0;
		unregister_chrdev(majordev, "cpu/msr_batch");
	}
}

static char *msrbatch_nodename(struct device *dev, umode_t *mode)
{
	return kasprintf(GFP_KERNEL, "cpu/msr_batch");
}

int msrbatch_init(void)
{
	int err;
	struct device *dev;

	majordev = register_chrdev(0, "cpu/msr_batch", &fops);
	if (majordev < 0) {
		pr_err("msrbatch_init: unable to register chrdev\n");
		msrbatch_cleanup();
		return -EBUSY;
	}
	cdev_registered = 1;

	cdev_class = class_create(THIS_MODULE, "msr_batch");
	if (IS_ERR(cdev_class)) {
		err = PTR_ERR(cdev_class);
		msrbatch_cleanup();
		return err;
	}
	cdev_class_created = 1;

	cdev_class->devnode = msrbatch_nodename;

	dev = device_create(cdev_class, NULL, MKDEV(majordev, 0),
						NULL, "msr_batch");
	if (IS_ERR(dev)) {
		err = PTR_ERR(dev);
		msrbatch_cleanup();
		return err;
	}
	cdev_created = 1;
	return 0;
}
