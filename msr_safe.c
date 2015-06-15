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
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cpu.h>
#include <linux/uaccess.h>
#include <linux/ctype.h>
#include <linux/hashtable.h>
#include <linux/slab.h>
#include <linux/mutex.h>

/* -- Begin setup for driver initialization and registration ---------------- */

/*
 * The minor device scheme works as follows:
 * 	0 : (NR_CPUS-1) are for the CPUs
 * 	NR_CPUS is the administrative whitelist interface
 */
#define MSR_NUM_MINORS NR_CPUS+1
#define MSR_WLIST_ADMIN_MINOR NR_CPUS
#define MSR_IS_WLIST_ADMIN(m) ((m) == MSR_WLIST_ADMIN_MINOR)

static struct class *msr_class;
static int majordev;
static char msr_chardev_created[MSR_NUM_MINORS];
static char msr_chardev_registered = 0;
static char msr_class_created = 0;
static char msr_notifier_registered = 0;

#define MAX_WLIST_BSIZE ((128 * 1024) + 1) /* "+1" for null character */

struct whitelist_entry {
	u64 wmask;	/* Bits that may be written */
	u64 rmask;	/* Bits that may be read */
	u64 msr;	/* Address of msr (used as hash key) */
	struct hlist_node hlist;
};

static DEFINE_HASHTABLE(msrwl_hash, 6);
/*TODO: (potentially) replace msrwl_mutex with RCU for faster reader access*/
static DEFINE_MUTEX(msrwl_mutex);
static struct whitelist_entry *msrwl_entries=0; /* white list entries array */
static int msrwl_numentries = 0;

static void msrwl_delete(void)
{
	if (msrwl_entries != 0) {
		kfree(msrwl_entries);
		msrwl_entries = 0;
		msrwl_numentries = 0;
	}
}

static int msrwl_create(int nentries)
{
	hash_init(msrwl_hash);
	msrwl_delete();
	msrwl_numentries = nentries;
	msrwl_entries = kmalloc((nentries * sizeof(struct whitelist_entry)), 
								GFP_KERNEL);

	if (!msrwl_entries) {
		printk(KERN_ALERT "msrwl_create: %lu byte allocation failed\n",
				(nentries * sizeof(struct whitelist_entry)));
		return -ENOMEM;
	}
	return 0;
}

static struct whitelist_entry *msrwl_find(u64 msr)
{
	struct whitelist_entry *entry;

	hash_for_each_possible(msrwl_hash, entry, hlist, msr)
	if (entry->msr == msr)
		return entry;
	return 0;
}

static void msrwl_add(struct whitelist_entry *entry)
{
	hash_add(msrwl_hash, &entry->hlist, entry->msr);
}

static u64 msrwl_get_readmask(loff_t reg)
{
	struct whitelist_entry *entry = msrwl_find((u64)reg);

	return entry ? entry->rmask : 0;
}

static u64 msrwl_get_writemask(loff_t reg)
{
	struct whitelist_entry *entry = msrwl_find((u64)reg);

	return entry ? entry->wmask : 0;
}

static int msrwl_parse_entry(char *inbuf, char **nextinbuf, 
					struct whitelist_entry *entry)
{
	char *s = skip_spaces(inbuf);
	int i;
	u64 data[3];

	while (*s == '#') { /* Skip remaining portion of line */
		for (s = s + 1; *s && *s != '\n'; s++)
			;
		s = skip_spaces(s);
	}

	if (*s == 0)
		return 0; /* This means we are done with the input buffer */

	for (i = 0; i < 3; i++) {/* we should have the first of 3 #s now */
		char *s2; 
		int err;
		char tmp;

		s2 = s = skip_spaces(s);
		while (!isspace(*s) && *s)
			s++;

		if (*s == 0) {
			printk(KERN_ALERT "msrwl_parse_entry: Premature EOF");
			return -EINVAL;
		}

		tmp = *s;
		*s = 0; /* Null-terminate this portion of string */
		if ((err = kstrtoull(s2, 0, &data[i]))) {
			printk(KERN_ALERT 
				"msrwl_parse_entry kstrtoull(%s) err(%d)\n", 
				s2, err);
			return err;
		}
		*s++ = tmp;
	}

	if (entry) {
		entry->msr = data[0];
		entry->wmask = data[1];
		entry->rmask = data[2];
	}

	*nextinbuf = s; /* Return where we left off to caller */
	return *nextinbuf - inbuf;
}

static ssize_t msrwl_update(struct file *file, const char __user *buf, 
						size_t count, loff_t *ppos)
{
	int err = 0;
	const u32 __user *tmp = (const u32 __user *)buf;
	char *s;
	int res;
	int num_entries;
	struct whitelist_entry *entry;
	char *msrbuf;

	/* TODO: Remove this restriction and handle large files like a man... */
	if (count+1 > MAX_WLIST_BSIZE) {
		printk(KERN_ALERT 
			"msrwl_update: Data buffer of %zu bytes too large\n",
		count);
		err = -EINVAL;
		goto out1;
	}

	msrbuf = kmalloc(count+1, GFP_KERNEL);
	if (!msrbuf) {
		printk(KERN_ALERT 
			"msrwl_update: failed to allocate buffer(%zu)\n", 
			count+1);
		err = -ENOMEM;
		goto out1;
	}

	if (copy_from_user(msrbuf, tmp, count)) {
		printk(KERN_ALERT 
			"msrwl_update: copy_from_user(%zu bytes) failed\n", 
		count);
		err = -EFAULT;
		goto out2;
	}

	msrbuf[count] = 0; // NULL-terminate to make it into a big string

	/*
	 * We make two passes through the file. The first pass is to ensure that
	 * the input file is valid. If the file is valid, we will then delete 
	 * the current white list and then perform the second pass to actually 
	 * create the new white list.
	 *
	 * Pass 1: 
	 */
	for (num_entries = 0, s = msrbuf, res = 1; res > 0; ) {
		if ((res = msrwl_parse_entry(s, &s, 0)) < 0) {
			err = res;
			goto out2;
		}

		if (res)
			num_entries++;
	}

	mutex_lock(&msrwl_mutex);
	if ((res = msrwl_create(num_entries)) < 0) {
		err = res;
		goto out3;
	}

	/* Pass 2: */
	for (entry = msrwl_entries, s = msrbuf, res = 1; res > 0; entry++) {
		if ((res = msrwl_parse_entry(s, &s, entry)) < 0) {
			printk(KERN_ALERT "msrw_update: Table corrupted\n");
			msrwl_delete();
			err = res; /* This should not happen! */
			goto out3;
		}

		if (res) {
			if (msrwl_find(entry->msr)) {
				printk(KERN_ALERT 
				   "msrw_update: Duplicate entry found: %llx\n",
					 entry->msr);
				err = -EINVAL;
				msrwl_delete();
				goto out3;
			}
			msrwl_add(entry);
		}
	}

out3:
	mutex_unlock(&msrwl_mutex);
out2:
	kfree(msrbuf);
out1:
	return err ? err : count;
}

static loff_t msr_seek(struct file *file, loff_t offset, int orig)
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

static ssize_t 
msr_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	u32 __user *tmp = (u32 __user *) buf;
	u32 data[2];
	u64 wl_readmask; /* Bits we are allowed to read */
	loff_t reg = *ppos;
	int cpu = iminor(file->f_path.dentry->d_inode);
	int err = -EACCES; /* Initialize to Permission Denied */

	if (MSR_IS_WLIST_ADMIN(cpu)) {
		/*TODO: Update read of whitelist to return enum*/
		return count-1;
	}

	/* "Count" doesn't have any meaning here, as we
	 * never want to read more than one msr at at time.
	 */
	if (count != 8)
		return -EINVAL;	/* Invalid chunk size */

	mutex_lock(&msrwl_mutex);
	wl_readmask = msrwl_get_readmask(reg);
	mutex_unlock(&msrwl_mutex);

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

static ssize_t 
msr_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	const u32 __user *tmp = (const u32 __user *)buf;
	u32 data[2];
	u64 *pdata = (u64*)&data[0];
	u64 wl_writemask;
	loff_t reg = *ppos;
	int cpu = iminor(file->f_path.dentry->d_inode);
	int err = -EACCES;

	if (MSR_IS_WLIST_ADMIN(iminor(file->f_path.dentry->d_inode)))
		return msrwl_update(file, buf, count, ppos);

	/* "Count" doesn't have any meaning here, as we
	 * never want to write more than one msr at at time.
	 */
	if (count != 8)
		return -EINVAL;	/* Invalid chunk size */

	mutex_lock(&msrwl_mutex);
	wl_writemask = msrwl_get_writemask(reg);
	mutex_unlock(&msrwl_mutex);

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

static int msr_open(struct inode *inode, struct file *file)
{
	unsigned int cpu;
	struct cpuinfo_x86 *c;

	cpu = iminor(file->f_path.dentry->d_inode);

	if (MSR_IS_WLIST_ADMIN(cpu))
		return 0; // Let 'em in...

	if (cpu >= nr_cpu_ids || !cpu_online(cpu))
		return -ENXIO; // No such CPU

	c = &cpu_data(cpu);
	if (!cpu_has(c, X86_FEATURE_MSR))
		return -EIO;	/* MSR not supported */

	return 0;
}

/*
 * File operations we support
 */
static const struct file_operations msr_fops = {
	.owner = THIS_MODULE,
	.read = msr_read,
	.write = msr_write,
	.open = msr_open,
	.llseek = msr_seek
};

static int __cpuinit msr_device_create(int cpu)
{
	struct device *dev;

	dev = device_create(msr_class, NULL, MKDEV(majordev, cpu), 
	NULL, "msr_safe%d", cpu);
	return IS_ERR(dev) ? PTR_ERR(dev) : 0;
}

static void msr_device_destroy(int cpu)
{
	device_destroy(msr_class, MKDEV(majordev, cpu));
}

static int __cpuinit 
msr_class_cpu_callback(struct notifier_block *nfb, 
					unsigned long action, void *hcpu)
{
	unsigned int cpu = (unsigned long)hcpu;
	int err = 0;

	switch (action)
	{
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
	.notifier_call = msr_class_cpu_callback
};

static char *msr_devnode(struct device *dev, umode_t *mode)
{
	if (MSR_IS_WLIST_ADMIN(MINOR(dev->devt)))
		return kasprintf(GFP_KERNEL, "cpu/msr_whitelist");
	else
		return kasprintf(GFP_KERNEL,"cpu/%u/msr_safe",MINOR(dev->devt));
}

static void msr_cleanup(void)
{
	int cpu = 0;

	msrwl_delete();

	if (msr_notifier_registered) {
		msr_notifier_registered = 0;
		unregister_hotcpu_notifier(&msr_class_cpu_notifier);
	}

	for_each_online_cpu(cpu) {
		if (msr_chardev_created[cpu]) {
			msr_chardev_created[cpu] = 0;
			msr_device_destroy(cpu);
		}
	}

	if (msr_chardev_created[MSR_WLIST_ADMIN_MINOR]) {
		msr_chardev_created[MSR_WLIST_ADMIN_MINOR] = 0;
		msr_device_destroy(MSR_WLIST_ADMIN_MINOR);
	}

	if (msr_class_created) {
		msr_class_created = 0;
		class_destroy(msr_class);
	}

	if (msr_chardev_registered) {
		msr_chardev_registered = 0;
		__unregister_chrdev(majordev,0, MSR_NUM_MINORS, "cpu/msr_safe");
	}
}

static int __init msr_init(void)
{
	int i, err;

	majordev = __register_chrdev(0,0,MSR_NUM_MINORS, 
					"cpu/msr_safe", &msr_fops);
	if (majordev < 0) {
		printk(KERN_ERR "msr_safe: unable to register device number\n");
		msr_cleanup();
		return -EBUSY;
	}
	msr_chardev_registered = 1;

	msr_class = class_create(THIS_MODULE, "msr_safe");
	if (IS_ERR(msr_class)) {
		err = PTR_ERR(msr_class);
		msr_cleanup();
		return err;
	}
	msr_class_created = 1;

	for (i = 0; i < MSR_NUM_MINORS; i++)
		msr_chardev_created[i] = 0;

	msr_class->devnode = msr_devnode;
	if ((err = msr_device_create(MSR_WLIST_ADMIN_MINOR))) {
		msr_cleanup();
		return err;
	}
	msr_chardev_created[MSR_WLIST_ADMIN_MINOR] = 1;

	i = 0;
	for_each_online_cpu(i) {
		if ((err = msr_device_create(i)) != 0) {
			msr_cleanup();
			return err;
		}
		msr_chardev_created[i] = 1;
	}

	register_hotcpu_notifier(&msr_class_cpu_notifier);
	msr_notifier_registered = 1;
	return 0;
}

static void __exit msr_exit(void)
{
	msr_cleanup();
}

module_init(msr_init);
module_exit(msr_exit)

MODULE_AUTHOR("Marty McFadden <mcfadden8@llnl.gov>");
MODULE_DESCRIPTION("x86 sanitized MSR driver");
MODULE_LICENSE("GPL");
MODULE_SUPPORTED_DEVICE("msr_safe");
/*TODO: Produce version information for this driver*/
/*TODO: Check version information of Linux kernel for base level*/

