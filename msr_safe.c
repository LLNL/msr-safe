/* ----------------------------------------------------------------------- *
 * Copyright 2000-2008 H. Peter Anvin - All Rights Reserved
 * Copyright 2009 Intel Corporation; author: H. Peter Anvin
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, Inc., 675 Mass Ave, Cambridge MA 02139,
 * USA; either version 2 of the License, or (at your option) any later
 * version; incorporated herein by reference.
 * ----------------------------------------------------------------------- */

/*
 * x86 MSR access device
 *
 * This device is accessed by lseek() to the appropriate register number
 * and then read/write in chunks of 8 bytes. A larger size means multiple
 * reads or writes of the same register.
 *
 * This driver uses /dev/cpu/%d/msr_safe where %d is the minor number, and on
 * an SMP box will direct the access to CPU %d.
 */
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cpu.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include "msr_safe.h"
#include "msr-whitelist.h"
#include "msr.h"

MODULE_AUTHOR("Marty McFadden <mcfadden8@llnl.gov>");
MODULE_DESCRIPTION("x86 sanitized MSR driver");
MODULE_LICENSE("GPL");
MODULE_SUPPORTED_DEVICE("msr_safe");

static int majordev;
static struct class *cdev_class;
static char cdev_created[MSR_NUM_MINORS];
static char cdev_registered = 0;
static char cdev_class_created = 0;
static char hotcpu_notifier_registered = 0;

/* -- Open/Close/Seek/Read/Write/Ioctl -- */
static int msr_safe_open(struct inode *inode, struct file *file)
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

static loff_t msr_safe_seek(struct file *file, loff_t offset, int orig)
{
	loff_t ret;
	struct inode *inode = file->f_mapping->host;

	mutex_lock(&inode->i_mutex);
	switch (orig)
	{
	case 0:
		file->f_pos = offset;
		ret = file->f_pos;
		break;
	case 1:
		file->f_pos += offset;
		ret = file->f_pos;
		break;
	default:
		ret = -EINVAL;
	}
	mutex_unlock(&inode->i_mutex);
	return ret;
}

static ssize_t msr_safe_read(struct file *file, char __user *buf, 
						size_t count, loff_t *ppos)
{
	u32 __user *tmp = (u32 __user *) buf;
	u32 data[2];
	u64 wl_readmask; /* Bits we are allowed to read */
	loff_t reg = *ppos;
	int cpu = iminor(file->f_path.dentry->d_inode);
	int err = -EACCES; /* Initialize to Permission Denied */

	/* 
	 * "count" doesn't have any meaning here, as we never want to read 
	 * more than one msr at at time.
	 */
	if (count != 8)
		return -EINVAL;	/* Invalid chunk size */

	wl_readmask = msr_whitelist_readmask(reg);

	if (wl_readmask == 0)
		return -EACCES;

	err = rdmsr_safe_on_cpu(cpu, reg, &data[0], &data[1]);
	if (!err) {
		u64 *pdata = (u64*)&data[0];

		*pdata &= wl_readmask;

		/* Only return bits that white list says we have acccess to */
		if (copy_to_user(tmp, data, 8))
			err = -EFAULT;
		else
			err = 0; /* Success */
	}
	return err ? err : 8;
}

static ssize_t msr_safe_write(struct file *file, 
		const char __user *buf, size_t count, loff_t *ppos)
{
	const u32 __user *tmp = (const u32 __user *)buf;
	u32 data[2];
	u64 *pdata = (u64*)&data[0];
	u64 wl_writemask;
	loff_t reg = *ppos;
	int cpu = iminor(file->f_path.dentry->d_inode);
	int err = -EACCES;

	if (count != 8)
		return -EINVAL;	/* Invalid chunk size */

	wl_writemask = msr_whitelist_writemask(reg);

	if (wl_writemask == 0)
		return -EACCES;

	if (copy_from_user(&data[0], tmp, 8)) {
		err = -EFAULT;
	}
	else {
		*pdata &= wl_writemask;
		err = wrmsr_safe_on_cpu(cpu, reg, data[0], data[1]);
	}
	return err ? err : 8;
}

static long msr_safe_ioctl(struct file *file, 
				unsigned int ioc, unsigned long arg)
{
	int err = 0;

	switch (ioc) {
	case X86_IOC_RDMSR_REGS:
	{
		u32 __user *uregs = (u32 __user *)arg;
		u32 regs[8];
		int cpu = iminor(file->f_path.dentry->d_inode);

		if (!(file->f_mode & FMODE_READ)) {
			err = -EBADF;
			break;
		}
		if (copy_from_user(&regs, uregs, sizeof regs)) {
			err = -EFAULT;
			break;
		}
		err = rdmsr_safe_regs_on_cpu(cpu, regs);
		if (err)
			break;
		if (copy_to_user(uregs, &regs, sizeof regs))
			err = -EFAULT;
		break;
	}

	case X86_IOC_WRMSR_REGS:
	{
		u32 __user *uregs = (u32 __user *)arg;
		u32 regs[8];
		int cpu = iminor(file->f_path.dentry->d_inode);

		if (!(file->f_mode & FMODE_WRITE)) {
			err = -EBADF;
			break;
		}
		if (copy_from_user(&regs, uregs, sizeof regs)) {
			err = -EFAULT;
			break;
		}
		err = wrmsr_safe_regs_on_cpu(cpu, regs);
		if (err)
			break;
		if (copy_to_user(uregs, &regs, sizeof regs))
			err = -EFAULT;
		break;
	}

	case X86_IOC_RDMSR_BATCH:
	{
		struct msr_bundle_desc __user *u_bdes;
		struct msr_bundle_desc k_bdes;
		struct msr_cpu_ops __user *u_bundle;

		if (!(file->f_mode & FMODE_READ)) {
			err = -EBADF;
			break;
		}
		u_bdes = (struct msr_bundle_desc *)arg;

		if (copy_from_user(&k_bdes, u_bdes, sizeof k_bdes)) {
			printk(KERN_ERR "RDMSR_BATCH: copyin(u_bdes) failed\n");
			err = -EFAULT;
			break;
		}

		if (k_bdes.n_msr_bundles <= 0) {
			printk(KERN_ERR "RDMSR_BATCH: Invalid size %d\n", 
							k_bdes.n_msr_bundles);
			err = -EINVAL;
			break;
		}
		u_bundle = k_bdes.bundle;
		k_bdes.bundle = kzalloc(k_bdes.n_msr_bundles * 
					sizeof(*k_bdes.bundle), GFP_KERNEL);
		if (!k_bdes.bundle) {
			printk(KERN_ERR 
				"RDMSR_BATCH: kzalloc(%lu) Failed", 
				k_bdes.n_msr_bundles * sizeof(*k_bdes.bundle));
			err = -ENOMEM;
			break;
		}
		
		if (copy_from_user(k_bdes.bundle, u_bundle, 
			      k_bdes.n_msr_bundles * sizeof(*k_bdes.bundle))) {
			printk(KERN_ERR "RDMSR_BATCH: copyin(bundle) Failed");
			err = -EFAULT;
			goto k_bundle_alloc;
		}

		if (k_bdes.n_msr_bundles <= 0) {
			printk(KERN_ERR "RDMSR_BATCH: Invalid size %d\n", 
							k_bdes.n_msr_bundles);
			err = -EINVAL;
			goto k_bundle_alloc;
		}

		/* TODO: Check against whitelist */

		if ((err = rdmsr_safe_bundle(&k_bdes)) != 0) {
			printk(KERN_ERR 
				"RDMSR_BATCH: rdmsr_safe_bundle: %d", err);
			goto k_bundle_alloc;
		}

		/* TODO: Apply whitelist */

		if (copy_to_user(u_bundle, k_bdes.bundle, 
				k_bdes.n_msr_bundles * sizeof(*u_bundle))) {
			printk(KERN_ERR "RDMSR_BATCH: copyout(bundle) Failed");
			err = -EFAULT;
			goto k_bundle_alloc;
		}
k_bundle_alloc:
		kfree(k_bdes.bundle);
		break;
	}
	default:
		err = -ENOTTY;
		break;
	}

	return err;
}

/*
 * File operations we support
 */
static const struct file_operations msr_safe_fops = {
	.owner = THIS_MODULE,
	.read = msr_safe_read,
	.write = msr_safe_write,
	.open = msr_safe_open,
	.llseek = msr_safe_seek,
	.unlocked_ioctl = msr_safe_ioctl,
	.compat_ioctl = msr_safe_ioctl
};

static int __cpuinit create_msr_safe_device(int cpu)
{
	struct device *dev;

	dev = device_create(cdev_class, NULL, MKDEV(majordev, cpu), 
	NULL, "msr_safe%d", cpu);
	return IS_ERR(dev) ? PTR_ERR(dev) : 0;
}

static void destroy_msr_safe_device(int cpu)
{
	device_destroy(cdev_class, MKDEV(majordev, cpu));
}

static int __cpuinit 
cdev_class_cpu_callback(struct notifier_block *nfb, 
					unsigned long action, void *hcpu)
{
	unsigned int cpu = (unsigned long)hcpu;
	int err = 0;

	switch (action)
	{
	case CPU_UP_PREPARE:
		err = create_msr_safe_device(cpu);
		break;
	case CPU_UP_CANCELED:
	case CPU_UP_CANCELED_FROZEN:
	case CPU_DEAD:
		destroy_msr_safe_device(cpu);
		break;
	}
	return notifier_from_errno(err);
}

static char *msr_safe_version = "1.8";

static ssize_t version_show(struct kobject *kobj, struct kobj_attribute *attr,
			    char *buf)
{
	return sprintf(buf, "%s\n", msr_safe_version);
}

static ssize_t version_set(struct kobject *kobj, struct kobj_attribute *attr,
			   const char *buf, size_t count)
{
	return count;
}

/* Sysfs attributes cannot be world-writable. */
static struct kobj_attribute version_attribute = 
	__ATTR(version, 0444, version_show, version_set);

static struct attribute *attrs[] = {
	&version_attribute.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};

static struct kobject *msr_safe_kobj;

static struct notifier_block __refdata cdev_class_cpu_notifier = {
	.notifier_call = cdev_class_cpu_callback
};

static char *msr_safe_nodename(struct device *dev, umode_t *mode)
{
	return kasprintf(GFP_KERNEL,"cpu/%u/msr_safe",MINOR(dev->devt));
}

static void msr_safe_cleanup(void)
{
	int cpu = 0;

	msr_whitelist_cleanup();

	if (msr_safe_kobj) {
		kobject_put(msr_safe_kobj);
		msr_safe_kobj = 0;
	}

	if (hotcpu_notifier_registered) {
		hotcpu_notifier_registered = 0;
		unregister_hotcpu_notifier(&cdev_class_cpu_notifier);
	}

	for_each_online_cpu(cpu) {
		if (cdev_created[cpu]) {
			cdev_created[cpu] = 0;
			destroy_msr_safe_device(cpu);
		}
	}

	if (cdev_class_created) {
		cdev_class_created = 0;
		class_destroy(cdev_class);
	}

	if (cdev_registered) {
		cdev_registered = 0;
		__unregister_chrdev(majordev,0, MSR_NUM_MINORS, "cpu/msr_safe");
	}
}

static int __init msr_safe_init(void)
{
	int i, err;

	if ((err = msr_whitelist_init()) != 0) {
		printk(KERN_ERR "msr_safe: failed to init whitelist\n");
		return err;
	}

	majordev = __register_chrdev(0,0,MSR_NUM_MINORS, 
					"cpu/msr_safe", &msr_safe_fops);
	if (majordev < 0) {
		printk(KERN_ERR "msr_safe: unable to register device number\n");
		msr_safe_cleanup();
		return -EBUSY;
	}
	cdev_registered = 1;

	cdev_class = class_create(THIS_MODULE, "msr_safe");
	if (IS_ERR(cdev_class)) {
		err = PTR_ERR(cdev_class);
		msr_safe_cleanup();
		return err;
	}
	cdev_class_created = 1;

	for (i = 0; i < MSR_NUM_MINORS; i++)
		cdev_created[i] = 0;

	cdev_class->devnode = msr_safe_nodename;

	i = 0;
	for_each_online_cpu(i) {
		if ((err = create_msr_safe_device(i)) != 0) {
			msr_safe_cleanup();
			return err;
		}
		cdev_created[i] = 1;
	}

	register_hotcpu_notifier(&cdev_class_cpu_notifier);
	hotcpu_notifier_registered = 1;

	msr_safe_kobj = kobject_create_and_add("msr_safe", kernel_kobj);
	if (!msr_safe_kobj)
		return -ENOMEM;

	err = sysfs_create_group(msr_safe_kobj, &attr_group);
	if (err)
		kobject_put(msr_safe_kobj);

	return 0;
}

static void __exit msr_safe_exit(void)
{
	msr_safe_cleanup();
}

module_init(msr_safe_init);
module_exit(msr_safe_exit)

