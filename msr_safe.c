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
#include <linux/list.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cpu.h>
#include <linux/uaccess.h>
#include <linux/ctype.h>
#include <linux/hashtable.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>
#include "msr_safe.h"

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
static DEFINE_HASHTABLE(whitelist_hash, 6);
static DEFINE_MUTEX(whitelist_mutex);	/*TODO (potentially) replace with RCU */
static struct whitelist_entry *whitelist=0;
static int whitelist_numentries = 0;

/* -- Open/Close/Seek/Read/Write/Ioctl -- */
static int msr_safe_open(struct inode *inode, struct file *file)
{
	unsigned int cpu;
	struct cpuinfo_x86 *c;

	cpu = iminor(file->f_path.dentry->d_inode);

	if (MSR_IS_WLIST_ADMIN(cpu))
		return 0; /* Let 'em in... */

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

	if (MSR_IS_WLIST_ADMIN(cpu))
		return read_whitelist(file, buf, count, ppos);

	/* 
	 * "count" doesn't have any meaning here, as we never want to read 
	 * more than one msr at at time.
	 */
	if (count != 8)
		return -EINVAL;	/* Invalid chunk size */

	mutex_lock(&whitelist_mutex);
	wl_readmask = get_readmask(reg);
	mutex_unlock(&whitelist_mutex);

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

/* -- White list I/O -- */
/*
 * After copying data from user spaec, we make two passes through the file. 
 * The first pass is to ensure that the input file is valid. If the file is 
 * valid, we will then delete the current white list and then perform the 
 * second pass to actually create the new white list.
 */
static ssize_t write_whitelist(struct file *file, const char __user *buf, 
						size_t count, loff_t *ppos)
{
	int err = 0;
	const u32 __user *tmp = (const u32 __user *)buf;
	char *s;
	int res;
	int num_entries;
	struct whitelist_entry *entry;
	char *kbuf;


	if (count <= 2) {
		mutex_lock(&whitelist_mutex);
		delete_whitelist();
		hash_init(whitelist_hash);
		mutex_unlock(&whitelist_mutex);
		return count;
	}

	if (count+1 > MAX_WLIST_BSIZE) {
		printk(KERN_ALERT 
		    "write_whitelist: Data buffer of %zu bytes too large\n",
		    count);
		return -EINVAL;
	}

	if (!(kbuf = kzalloc(count+1, GFP_KERNEL)))
		return -ENOMEM;

	if (copy_from_user(kbuf, tmp, count)) {
		err = -EFAULT;
		goto out_freebuffer;
	}

	/* Pass 1: */
	for (num_entries = 0, s = kbuf, res = 1; res > 0; ) {
		if ((res = parse_next_whitelist_entry(s, &s, 0)) < 0) {
			err = res;
			goto out_freebuffer;
		}

		if (res)
			num_entries++;
	}

	/* Pass 2: */
	mutex_lock(&whitelist_mutex);
	if ((res = create_whitelist(num_entries)) < 0) {
		err = res;
		goto out_releasemutex;
	}

	for (entry = whitelist, s = kbuf, res = 1; res > 0; entry++) {
		if ((res = parse_next_whitelist_entry(s, &s, entry)) < 0) {
			printk(KERN_ALERT "msrw_update: Table corrupted\n");
			delete_whitelist();
			err = res; /* This should not happen! */
			goto out_releasemutex;
		}

		if (res) {
			if (find_in_whitelist(entry->msr)) {
				printk(KERN_ALERT 
				   "msrw_update: Duplicate entry found: %llx\n",
					 entry->msr);
				err = -EINVAL;
				delete_whitelist();
				goto out_releasemutex;
			}
			add_to_whitelist(entry);
		}
	}

out_releasemutex:
	mutex_unlock(&whitelist_mutex);
out_freebuffer:
	kfree(kbuf);
	return err ? err : count;
}

static ssize_t read_whitelist(struct file *file, char __user *buf, 
						size_t count, loff_t *ppos)
{
	loff_t idx = *ppos;
	u32 __user *tmp = (u32 __user *) buf;
	char kbuf[160];
	int len;
	struct whitelist_entry entry;

	mutex_lock(&whitelist_mutex);
	*ppos = 0;

	if (idx >= whitelist_numentries || idx < 0) {
		mutex_unlock(&whitelist_mutex);
		return 0;
	}

	entry = whitelist[idx];
	mutex_unlock(&whitelist_mutex);

	len = sprintf(kbuf, 
		"MSR: %08llx Write Mask: %016llx Read Mask: %016llx\n", 
					entry.msr, entry.wmask, entry.rmask);

	if (len > count)
		return -EFAULT;

	if (copy_to_user(tmp, kbuf, len))
		return -EFAULT;

	*ppos = idx+1;
	return len;
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

	if (MSR_IS_WLIST_ADMIN(iminor(file->f_path.dentry->d_inode)))
		return write_whitelist(file, buf, count, ppos);

	/* "Count" doesn't have any meaning here, as we
	 * never want to write more than one msr at at time.
	 */
	if (count != 8)
		return -EINVAL;	/* Invalid chunk size */

	mutex_lock(&whitelist_mutex);
	wl_writemask = get_writemask(reg);
	mutex_unlock(&whitelist_mutex);

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

static void delete_whitelist(void)
{
	struct whitelist_entry *e;

	if (whitelist == 0)
		return;

	for (e = whitelist; e < whitelist+whitelist_numentries; e++) {
		if (CLEAR_MSR_ON_WHITELIST_REMOVE(e->resflag)) {
			/* Clear out the MSR */
		}
		if (RESTORE_MSR_ON_WHITELIST_REMOVE(e->resflag)) {
			/* Restore bits that we can */
		}
	}

	if (whitelist->msrdata != 0)
		kfree(whitelist->msrdata);

	kfree(whitelist);
	whitelist = 0;
	whitelist_numentries = 0;
}

static int create_whitelist(int nentries)
{
	hash_init(whitelist_hash);
	delete_whitelist();
	whitelist_numentries = nentries;
	whitelist = kcalloc(nentries, sizeof(*whitelist), GFP_KERNEL);

	if (!whitelist) {
		printk(KERN_ALERT 
			"create_whitelist: %lu byte allocation failed\n",
					(nentries * sizeof(*whitelist)));
		return -ENOMEM;
	}
	return 0;
}

static struct whitelist_entry *find_in_whitelist(u64 msr)
{
	struct whitelist_entry *entry = 0;

	if (whitelist) {
		hash_for_each_possible(whitelist_hash, entry, hlist, msr)
		if (entry && entry->msr == msr)
			return entry;
	}
	return 0;
}

static void add_to_whitelist(struct whitelist_entry *entry)
{
	hash_add(whitelist_hash, &entry->hlist, entry->msr);
}

static u64 get_readmask(loff_t reg)
{
	struct whitelist_entry *entry = find_in_whitelist((u64)reg);

	return entry ? entry->rmask : 0;
}

static u64 get_writemask(loff_t reg)
{
	struct whitelist_entry *entry = find_in_whitelist((u64)reg);

	return entry ? entry->wmask : 0;
}

static int parse_next_whitelist_entry(char *inbuf, char **nextinbuf, 
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
			printk(KERN_ALERT 
				"parse_next_whitelist_entry: Premature EOF");
			return -EINVAL;
		}

		tmp = *s;
		*s = 0; /* Null-terminate this portion of string */
		if ((err = kstrtoull(s2, 0, &data[i]))) {
			printk(KERN_ALERT 
			  "parse_next_whitelist_entry kstrtoull %s err=%d\n", 
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

/*
 * File operations we support
 */
static const struct file_operations msr_safe_fops = {
	.owner = THIS_MODULE,
	.read = msr_safe_read,
	.write = msr_safe_write,
	.open = msr_safe_open,
	.llseek = msr_safe_seek
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

static char *msr_safe_version = "1.7";

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
	if (MSR_IS_WLIST_ADMIN(MINOR(dev->devt)))
		return kasprintf(GFP_KERNEL, "cpu/msr_whitelist");
	else
		return kasprintf(GFP_KERNEL,"cpu/%u/msr_safe",MINOR(dev->devt));
}

static void msr_safe_cleanup(void)
{
	int cpu = 0;

	delete_whitelist();

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

	if (cdev_created[MSR_WLIST_ADMIN_MINOR]) {
		cdev_created[MSR_WLIST_ADMIN_MINOR] = 0;
		destroy_msr_safe_device(MSR_WLIST_ADMIN_MINOR);
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
	if ((err = create_msr_safe_device(MSR_WLIST_ADMIN_MINOR))) {
		msr_safe_cleanup();
		return err;
	}
	cdev_created[MSR_WLIST_ADMIN_MINOR] = 1;

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

