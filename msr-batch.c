/*
 * x86 MSR batch access device
 *
 * This device is accessed by ioctl() to submit a batch of MSR requests 
 * which may be used instead of or in addition to the lseek()/write()/read()
 * mechanism provided by msr_safe.c
 *
 * This driver uses /dev/cpu/msr_batch as its device file.
 */
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
#include "msr-whitelist.h"
#include "msr-batch.h"
#include "msr.h"

static int majordev;
static struct class *cdev_class;
static char cdev_created;
static char cdev_registered = 0;
static char cdev_class_created = 0;

static int msrbatch_open(struct inode *inode, struct file *file)
{
	unsigned int cpu;
	struct cpuinfo_x86 *c;

	cpu = iminor(file->f_path.dentry->d_inode);

	if (cpu >= nr_cpu_ids || !cpu_online(cpu))
		return -ENXIO; /* No such CPU */

	c = &cpu_data(cpu);
	if (!cpu_has(c, X86_FEATURE_MSR))
		return -EIO;	/* MSR not supported */

	return 0;
}

static int msr_batch_prefixup(struct msr_bundle_desc *bd)
{
	struct msr_cpu_ops *ops;
	struct msr_op *op;
	int err = 0;

	for (ops = bd->bundle; ops < bd->bundle + bd->n_msr_bundles; ++ops) {
		if (ops->n_ops <= 0 || ops->n_ops >= MSR_MAX_BATCH_OPS)
			return -EINVAL;

		for (op = &ops->ops[0]; op < &ops->ops[ops->n_ops]; ++op) {
			op->mask = op->isread ? 
					msr_whitelist_readmask(op->msr) : 
					msr_whitelist_writemask(op->msr);
			
			if (op->mask == 0) {
				printk(KERN_ERR
					"msr_prefixup: CPU %x MSR %x EPERM",
						             ops->cpu, op->msr);
				op->errno = err = -EPERM;
			} else {
				if (!op->isread)
					op->d.d64 &= op->mask;
				op->errno = 0;
			}
		}
	}

	return err;
}

static void msr_batch_postfixup(struct msr_bundle_desc *bd)
{
	struct msr_cpu_ops *ops;
	struct msr_op *op;

	for (ops = bd->bundle; ops < bd->bundle + bd->n_msr_bundles; ++ops)
		for (op = &ops->ops[0]; op < &ops->ops[ops->n_ops]; ++op)
			if (op->isread)
				op->d.d64 &= op->mask;
}

static long msrbatch_ioctl(struct file *f, unsigned int ioc, unsigned long arg)
{
	int err = 0;
	struct msr_bundle_desc __user *u_bdes;
	struct msr_bundle_desc k_bdes;
	struct msr_cpu_ops __user *u_bundle;

	if (ioc != X86_IOC_MSR_BATCH)
		return -ENOTTY;

	if ((f->f_mode & (FMODE_READ+FMODE_WRITE)) != (FMODE_READ+FMODE_WRITE))
		return -EBADF;

	u_bdes = (struct msr_bundle_desc *)arg;

	if (copy_from_user(&k_bdes, u_bdes, sizeof k_bdes))
		return -EFAULT;

	if (k_bdes.n_msr_bundles <= 0)
		return -EINVAL;

	u_bundle = k_bdes.bundle;
	k_bdes.bundle = kmalloc(k_bdes.n_msr_bundles * 
					sizeof(*k_bdes.bundle), GFP_KERNEL);
	if (!k_bdes.bundle)
		return -ENOMEM;
	
	if (k_bdes.n_msr_bundles <= 0) {
		err = -EINVAL;
		goto bundle_alloc;
	}

	if (copy_from_user(k_bdes.bundle, u_bundle, 
		      k_bdes.n_msr_bundles * sizeof(*k_bdes.bundle))) {
		err = -EFAULT;
		goto bundle_alloc;
	}

	if ((err = msr_batch_prefixup(&k_bdes)))
		goto copyout_and_return;


	if ((err = msr_safe_bundle(&k_bdes)) != 0)
		goto copyout_and_return;

	msr_batch_postfixup(&k_bdes);

copyout_and_return:
	if (copy_to_user(u_bundle, k_bdes.bundle, 
				k_bdes.n_msr_bundles * sizeof(*u_bundle))) {
		printk(KERN_ERR "MSR_BATCH: copyout(bundle) Failed");
		if (!err)
			err = -EFAULT;
	}
bundle_alloc:
	kfree(k_bdes.bundle);

	return err;
}

static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = msrbatch_open,
	.unlocked_ioctl = msrbatch_ioctl,
	.compat_ioctl = msrbatch_ioctl
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
	return kasprintf(GFP_KERNEL,"cpu/msr_batch");
}

int msrbatch_init(void)
{
	int err;
	struct device *dev;

	majordev = register_chrdev(0, "cpu/msr_batch", &fops);
	if (majordev < 0) {
		printk(KERN_ERR 
		    "msrbatch_init: unable to register chrdev\n");
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

