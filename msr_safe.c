/* ----------------------------------------------------------------------- *
 *
 *   Copyright 2000-2008 H. Peter Anvin - All Rights Reserved
 *   Copyright 2009 Intel Corporation; author: H. Peter Anvin
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, Inc., 675 Mass Ave, Cambridge MA 02139,
 *   USA; either version 2 of the License, or (at your option) any later
 *   version; incorporated herein by reference.
 *
 * ----------------------------------------------------------------------- */

/*
 * x86 MSR access device
 *
 * This device is accessed by lseek() to the appropriate register number
 * and then read/write in chunks of 8 bytes.  A larger size means multiple
 * reads or writes of the same register.
 *
 * This driver uses /dev/cpu/%d/msr_safe where %d is the minor number, and on
 * an SMP box will direct the access to CPU %d.
 */

/*Patki from Denver*/

#include <linux/module.h>

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/smp.h>
#include <linux/major.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cpu.h>
#include <linux/notifier.h>
#include <linux/uaccess.h>

#include <asm/processor.h>

#define _USE_ARCH_062D 1
#include "msr-supplemental.h"


//#define MSR_LAST_ENTRY ~0
//#define FAKE_LAST_MSR MSR_LAST_ENTRY, 0,0,0,0


static struct class *msr_class;
static int majordev;

#define SMSR_ENTRY(x,y) x
typedef enum smsr{
SMSR_ENTRIES
} smsr_type;
#undef SMSR_ENTRY


#define SMSR_ENTRY(x,y) y
static u16 whitelist[] = { SMSR_ENTRIES };
#undef SMSR_ENTRY

u16 get_whitelist_entry(u16 reg)
{
	u16 entry;

	for (entry = 0; entry < SMSR_LAST_ENTRY; entry++){
		if ( (whitelist[entry] & SMSR_REG_MASK) == reg){
			return reg;
		}
	}

	return 0;
}

static ssize_t msr_read(struct file *file, char __user *buf,
			size_t count, loff_t *ppos)
{
	u32 __user *tmp = (u32 __user *) buf;
	u32 data[2];
	u32 reg = *ppos;
	int cpu = iminor(file->f_path.dentry->d_inode);
	int err = 0;

	if (count != 8){
		return -EINVAL;	/* Invalid chunk size */
	}

	reg = get_whitelist_entry( (u16)(reg & SMSR_REG_MASK) );

	if(reg){
		err = rdmsr_safe_on_cpu(cpu, reg & SMSR_REG_MASK, &data[0], &data[1]);
		if (!err){
			if (copy_to_user(tmp, &data, 8)) {
				err = -EFAULT;
			}
		}
	}
	return err ? err : 8;
}

static ssize_t msr_write(struct file *file, const char __user *buf,
			 size_t count, loff_t *ppos)
{
	const u32 __user *tmp = (const u32 __user *)buf;
	u32 data[2];
	u32 reg = *ppos;
	int cpu = iminor(file->f_path.dentry->d_inode);
	int err = 0;

	if (count != 8)
		return -EINVAL;	/* Invalid chunk size */

	reg = get_whitelist_entry( (u16)(reg & SMSR_REG_MASK) );
	
	if(reg && ( reg & SMSR_RW_MASK ) ){
		if (copy_from_user(&data, tmp, 8)) {
			err = -EFAULT;
		}
		if(!err){
			err = wrmsr_safe_on_cpu(cpu, reg & SMSR_REG_MASK, data[0], data[1]);
		}
	}
	return err ? err : 8;
}


static int msr_open(struct inode *inode, struct file *file)
{
	unsigned int cpu;
	struct cpuinfo_x86 *c;
	int ret = 0;

	cpu = iminor(file->f_path.dentry->d_inode);

	if (cpu >= nr_cpu_ids || !cpu_online(cpu)) {
		ret = -ENXIO;	/* No such CPU */
		goto out;
	}
	c = &cpu_data(cpu);
	if (!cpu_has(c, X86_FEATURE_MSR))
		ret = -EIO;	/* MSR not supported */
out:
	return ret;
}

/*
 * File operations we support
 */
static const struct file_operations msr_fops = {
	.owner = THIS_MODULE,
	.read = msr_read,
	.write = msr_write,
	.open = msr_open,
};

static int __cpuinit msr_device_create(int cpu)
{
	struct device *dev;

	dev = device_create(msr_class, NULL, MKDEV(majordev, cpu), NULL,
			    "msr_safe%d", cpu);
	return IS_ERR(dev) ? PTR_ERR(dev) : 0;
}

static void msr_device_destroy(int cpu)
{
	device_destroy(msr_class, MKDEV(majordev, cpu));
}

static int __cpuinit msr_class_cpu_callback(struct notifier_block *nfb,
				unsigned long action, void *hcpu)
{
	unsigned int cpu = (unsigned long)hcpu;
	int err = 0;

	switch (action) {
	case CPU_UP_PREPARE:
		err = msr_device_create(cpu);
		break;
	case CPU_UP_CANCELED:
	case CPU_UP_CANCELED_FROZEN:
	case CPU_DEAD:
		msr_device_destroy(cpu);
		break;
	}
	return notifier_from_errno(err);
}

static struct notifier_block __refdata msr_class_cpu_notifier = {
	.notifier_call = msr_class_cpu_callback,
};

static char *msr_devnode(struct device *dev, mode_t *mode)
{
	return kasprintf(GFP_KERNEL, "cpu/%u/msr_safe", MINOR(dev->devt));
}

static int __init msr_init(void)
{
	int i, err = 0;
	i = 0;

	majordev = __register_chrdev(0, 0, NR_CPUS, "cpu/msr_safe", &msr_fops);
	if (majordev < 0) {
		printk(KERN_ERR "msr_safe: unable to register device number\n");
		err = -EBUSY;
		goto out;
	}
	msr_class = class_create(THIS_MODULE, "msr_safe");
	if (IS_ERR(msr_class)) {
		err = PTR_ERR(msr_class);
		goto out_chrdev;
	}
	msr_class->devnode = msr_devnode;
	for_each_online_cpu(i) {
		err = msr_device_create(i);
		if (err != 0)
			goto out_class;
	}
	register_hotcpu_notifier(&msr_class_cpu_notifier);

	err = 0;
	goto out;

out_class:
	i = 0;
	for_each_online_cpu(i)
		msr_device_destroy(i);
	class_destroy(msr_class);
out_chrdev:
	__unregister_chrdev(majordev, 0, NR_CPUS, "cpu/msr_safe");
out:
	return err;
}

static void __exit msr_exit(void)
{
	int cpu = 0;
	for_each_online_cpu(cpu)
		msr_device_destroy(cpu);
	class_destroy(msr_class);
	__unregister_chrdev(majordev, 0, NR_CPUS, "cpu/msr_safe");
	unregister_hotcpu_notifier(&msr_class_cpu_notifier);
}

module_init(msr_init);
module_exit(msr_exit)

MODULE_AUTHOR("Jim Foraker <foraker1@llnl.gov>");
MODULE_AUTHOR("Barry Rountree <rountree@llnl.gov>");
MODULE_DESCRIPTION("x86 sanitized MSR driver");
MODULE_LICENSE("GPL");
