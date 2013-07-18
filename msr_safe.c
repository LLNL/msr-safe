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
#include <linux/smp_lock.h>
#include <linux/major.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cpu.h>
#include <linux/notifier.h>
#include <linux/uaccess.h>

#include <asm/processor.h>
#include <asm/msr.h>
#include <asm/system.h>

#include "msr-supplemental.h"

static struct class *msr_class;
static int majordev;

struct msr_whitelist {
	u32	reg;
	u32	read_mask[2];
	u32	write_mask[2];
};

#define MASK_ALL {0xFFFFFFFF, 0xFFFFFFFF}
#define MASK_NONE {0, 0}
#define MSR_LAST_ENTRY ~0
#define MSR_ENTRY(reg, read, write) {reg, read, write}
#define MSR_RW(reg) MSR_ENTRY(reg, MASK_ALL, MASK_ALL)
#define MSR_RO(reg) MSR_ENTRY(reg, MASK_ALL, MASK_NONE)
#define MSR_RAPL_RW_DOMAIN(pkg) \
		MSR_RW(MSR_RAPL_POWER_LIMIT(pkg)), \
		MSR_RW(MSR_RAPL_ENERGY_STATUS(pkg)), \
		MSR_RW(MSR_RAPL_POLICY(pkg)), \
		MSR_RW(MSR_RAPL_PERF_STATUS(pkg)), \
		MSR_RW(MSR_RAPL_POWER_INFO(pkg))
#define MSR_RAPL_RW_DOMAIN(pkg) \
		 MSR_RW(MSR_RAPL_POWER_LIMIT(pkg)), \
		MSR_RW(MSR_RAPL_ENERGY_STATUS(pkg)), \
		MSR_RW(MSR_RAPL_POLICY(pkg)), \
		MSR_RW(MSR_RAPL_PERF_STATUS(pkg)), \
		MSR_RW(MSR_RAPL_POWER_INFO(pkg))

static struct msr_whitelist whitelist[] = {
	MSR_RO(MSR_IA32_P5_MC_ADDR),
	MSR_RO(MSR_IA32_P5_MC_TYPE),
	MSR_RO(MSR_IA32_TSC),
	MSR_RO(MSR_IA32_PLATFORM_ID),
	MSR_RW(MSR_IA32_MPERF),
	MSR_RW(MSR_IA32_APERF),
	MSR_RAPL_RW_DOMAIN(PKG),
	MSR_RAPL_RW_DOMAIN(DRAM),
	MSR_RAPL_RW_DOMAIN(PP0),
	MSR_RAPL_RW_DOMAIN(PP1),
	MSR_ENTRY(MSR_LAST_ENTRY, MASK_NONE, MASK_NONE)
};

static struct msr_whitelist *get_whitelist_entry(u64 reg)
{
	struct msr_whitelist *entry;

	for (entry = whitelist; entry->reg != MSR_LAST_ENTRY; entry++)
		if (entry->reg == reg)
			return entry;

	return NULL;
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
	u32 reg = *ppos;
	int cpu = iminor(file->f_path.dentry->d_inode);
	int err = 0;
	ssize_t bytes = 0;
	struct msr_whitelist *wlp;
	u32 read_mask[] = {0, 0};

	if (count % 8)
		return -EINVAL;	/* Invalid chunk size */

	wlp = get_whitelist_entry(reg);
	if (wlp) {
		read_mask[0] = wlp->read_mask[0];
		read_mask[1] = wlp->read_mask[1];
	}
	if (capable(CAP_SYS_RAWIO))
		read_mask[0] = read_mask[1] = ~0;

	if (!read_mask[0] && !read_mask[1])
		return -EINVAL;

	for (; count; count -= 8) {
		err = rdmsr_safe_on_cpu(cpu, reg, &data[0], &data[1]);
		if (err)
			break;
		data[0] &= read_mask[0];
		data[1] &= read_mask[1];
		if (copy_to_user(tmp, &data, 8)) {
			err = -EFAULT;
			break;
		}
		tmp += 2;
		bytes += 8;
	}

	return bytes ? bytes : err;
}

static ssize_t msr_write(struct file *file, const char __user *buf,
			 size_t count, loff_t *ppos)
{
	const u32 __user *tmp = (const u32 __user *)buf;
	u32 data[2];
	u32 reg = *ppos;
	int cpu = iminor(file->f_path.dentry->d_inode);
	int err = 0;
	ssize_t bytes = 0;
	struct msr_whitelist *wlp;
	u32 write_mask[] = { 0, 0};
	u32 orig_data[2];

	if (count % 8)
		return -EINVAL;	/* Invalid chunk size */

	wlp = get_whitelist_entry(reg);
	if (wlp) {
		write_mask[0] = wlp->write_mask[0];
		write_mask[1] = wlp->write_mask[1];
	}

	if (capable(CAP_SYS_RAWIO))
		write_mask[0] = write_mask[1] = ~0;

	if (!write_mask[0] && !write_mask[1])
		return -EINVAL;

	for (; count; count -= 8) {
		if (copy_from_user(&data, tmp, 8)) {
			err = -EFAULT;
			break;
		}
		if (~write_mask[0] || ~write_mask[1]) {
			err = rdmsr_safe_on_cpu(cpu, reg, &orig_data[0],
						&orig_data[1]);
			if (err)
				break;
			data[0] = (orig_data[0] & ~write_mask[0]) |
				  (data[0] & write_mask[0]);
			data[1] = (orig_data[1] & ~write_mask[1]) |
				  (data[1] & write_mask[1]);
		}
		err = wrmsr_safe_on_cpu(cpu, reg, data[0], data[1]);
		if (err)
			break;
		tmp += 2;
		bytes += 8;
	}

	return bytes ? bytes : err;
}

/* 
 * arg is a pointer to 8 u32's in the following format:
 * 	u32 arg[] = {eax, ecx, edx, ebc, esp, ebp, esi, edi};
 * 	Of which:
 * 		arg[1] (ecx) = msr number
 * 		arg[0] (eax) = low 32 bits of msr value
 * 		arg[2] (edx) = high 32 bits of msr value
 * 	These reg
 */
static long msr_ioctl(struct file *file, unsigned int ioc, unsigned long arg)
{
	u32 __user *uregs = (u32 __user *)arg;
	u32 regs[8], orig_regs[8];
	int cpu = iminor(file->f_path.dentry->d_inode);
	int err;
	struct msr_whitelist *wlp;
	u32 read_mask[] = {0, 0};
	u32 write_mask[] = {0, 0};

	if (capable(CAP_SYS_RAWIO)) {
		read_mask[0] = read_mask[1] = ~0;
		write_mask[0] = write_mask[1] = ~0;
	}
	
	switch (ioc) {
	case X86_IOC_RDMSR_REGS:
		if (!(file->f_mode & FMODE_READ)) {
			err = -EBADF;
			break;
		}
		if (copy_from_user(&regs, uregs, sizeof regs)) {
			err = -EFAULT;
			break;
		}
		wlp = get_whitelist_entry(regs[1]);
		if (wlp) {
			read_mask[0] |= wlp->read_mask[0];
			read_mask[1] |= wlp->read_mask[1];
		}
		if (!(read_mask[0] || read_mask[1])) {
			err = -EFAULT;
			break;
		}
		err = rdmsr_safe_regs_on_cpu(cpu, regs);
		regs[0] &= read_mask[0];
		regs[2] &= read_mask[1];
		if (err)
			break;
		if (copy_to_user(uregs, &regs, sizeof regs))
			err = -EFAULT;
		break;

	case X86_IOC_WRMSR_REGS:
		if (!(file->f_mode & FMODE_WRITE)) {
			err = -EBADF;
			break;
		}
		if (copy_from_user(&regs, uregs, sizeof regs)) {
			err = -EFAULT;
			break;
		}
		wlp = get_whitelist_entry(regs[1]);
		if (wlp) {
			read_mask[0] |= wlp->read_mask[0];
			read_mask[1] |= wlp->read_mask[1];
			write_mask[0] |= wlp->write_mask[0];
			write_mask[1] |= wlp->write_mask[1];
		}
		if (!(write_mask[0] || write_mask[1])) {
			err = -EFAULT;
			break;
		}
		if (~write_mask[0] || ~write_mask[1]) {
			memcpy(orig_regs, regs,sizeof(regs));
			err = rdmsr_safe_regs_on_cpu(cpu, orig_regs);
			if (err)
				break;
			regs[0] = (orig_regs[0] & ~write_mask[0]) |
				  (regs[0] & write_mask[0]);
			regs[2] = (orig_regs[2] & ~write_mask[1]) |
				  (regs[2] & write_mask[1]);
		}
		err = wrmsr_safe_regs_on_cpu(cpu, regs);
		regs[0] &= read_mask[0];
		regs[2] &= read_mask[1];
		if (err)
			break;
		if (copy_to_user(uregs, &regs, sizeof regs))
			err = -EFAULT;
		break;

	default:
		err = -ENOTTY;
		break;
	}

	return err;
}

static int msr_open(struct inode *inode, struct file *file)
{
	unsigned int cpu;
	struct cpuinfo_x86 *c;
	int ret = 0;

	lock_kernel();
	cpu = iminor(file->f_path.dentry->d_inode);

	if (cpu >= nr_cpu_ids || !cpu_online(cpu)) {
		ret = -ENXIO;	/* No such CPU */
		goto out;
	}
	c = &cpu_data(cpu);
	if (!cpu_has(c, X86_FEATURE_MSR))
		ret = -EIO;	/* MSR not supported */
out:
	unlock_kernel();
	return ret;
}

/*
 * File operations we support
 */
static const struct file_operations msr_fops = {
	.owner = THIS_MODULE,
	.llseek = msr_seek,
	.read = msr_read,
	.write = msr_write,
	.open = msr_open,
	.unlocked_ioctl = msr_ioctl,
	.compat_ioctl = msr_ioctl,
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
	__unregister_chrdev(majordev, 0, NR_CPUS, "cpu/msr");
out:
	return err;
}

static void __exit msr_exit(void)
{
	int cpu = 0;
	for_each_online_cpu(cpu)
		msr_device_destroy(cpu);
	class_destroy(msr_class);
	__unregister_chrdev(majordev, 0, NR_CPUS, "cpu/msr");
	unregister_hotcpu_notifier(&msr_class_cpu_notifier);
}

module_init(msr_init);
module_exit(msr_exit)

MODULE_AUTHOR("H. Peter Anvin <hpa@zytor.com>");
MODULE_DESCRIPTION("x86 sanitized MSR driver");
MODULE_LICENSE("GPL");
