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

#define _USE_ARCH_062D 
#include "msr-supplemental.h"
#undef _USE_ARCH_062D

static struct class *msr_class;
static int majordev;

struct smsr_entry{
	loff_t 	reg;		// Almost all MSR addresses are 16-bit values,
				//   including all the ones we care about.
				//   Use 32 bits for compatibility.

	u32	write_mask_0;	// Prevent writing to reserved bits and
	u32	write_mask_1;	//   reading/writing sensitive bits of
};

#define SMSR_ENTRY(a,b,c,d) a
typedef enum smsr{
SMSR_ENTRIES
} smsr_t;
#undef SMSR_ENTRY


#define SMSR_ENTRY(a,b,c,d) b,c,d
struct smsr_entry whitelist[] = { SMSR_ENTRIES };
#undef SMSR_ENTRY

u16 get_whitelist_entry(loff_t reg)
{
	smsr_t entry;
	for (entry = 0; entry < SMSR_LAST_ENTRY; entry++){
		if ( whitelist[entry].reg == reg){
			return entry;
		}
	}
	return 0;
}

static long msr_ioctl(struct file *file, unsigned int ioc, unsigned long arg)
{
	// Force access through read and write only.
	(void)file;
	(void)ioc;
	(void)arg;
	return -ENOTTY;
}

static loff_t msr_seek(struct file *file, loff_t offset, int orig)
{
        loff_t ret;
        struct inode *inode = file->f_mapping->host;

        mutex_lock(&inode->i_mutex);
        switch (orig) {
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


static ssize_t msr_read(struct file *file, char __user *buf,
			size_t count, loff_t *ppos)
{
	u32 __user *tmp = (u32 __user *) buf;
	u32 data[2];
	smsr_t idx;
	loff_t reg = *ppos;
	int cpu = iminor(file->f_path.dentry->d_inode);
	int err = 0;

	// "Count" doesn't have any meaning here, as we
	// never want to read more than one msr at at time.
	if (count != 8){
		return -EINVAL;	/* Invalid chunk size */
	}

	idx = get_whitelist_entry( reg );

	if(reg){
		err = rdmsr_safe_on_cpu(cpu, reg, &data[0], &data[1]);
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
	smsr_t idx;
	loff_t reg = *ppos;
	int cpu = iminor(file->f_path.dentry->d_inode);
	int err = 0;

	// "Count" doesn't have any meaning here, as we
	// never want to write more than one msr at at time.
	if (count != 8)
		return -EINVAL;	/* Invalid chunk size */

	idx = get_whitelist_entry( reg );
	
	// If the write masks are zero, don't bother writing.
	if(idx && (whitelist[idx].write_mask_0 | whitelist[idx].write_mask_1 )) {
		if (copy_from_user(&data, tmp, 8)) {
			err = -EFAULT;
		}
		if(!err){
			data[0] &= whitelist[idx].write_mask_0;
			data[1] &= whitelist[idx].write_mask_1;
			err = wrmsr_safe_on_cpu(cpu, reg, data[0], data[1]);
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
	.llseek = msr_seek,		// always returns -EINVAL
	.unlocked_ioctl = msr_ioctl,	// always returns -ENOTTY
	.compat_ioctl = msr_ioctl,	// always returns -ENOTTY
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
