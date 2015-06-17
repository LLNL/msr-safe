#ifndef MSR_SAFE_INC
#define MSR_SAVE_INC 1
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

/*
 * The minor device scheme works as follows:
 * 	0 : (NR_CPUS-1) are for the CPUs
 * 	NR_CPUS is the administrative whitelist interface
 */
#define MSR_NUM_MINORS NR_CPUS+1
#define MSR_WLIST_ADMIN_MINOR NR_CPUS
#define MSR_IS_WLIST_ADMIN(m) ((m) == MSR_WLIST_ADMIN_MINOR)
#define MAX_WLIST_BSIZE ((128 * 1024) + 1) /* "+1" for null character */

struct whitelist_entry {
	u64 wmask;	/* Bits that may be written */
	u64 rmask;	/* Bits that may be read */
	u64 msr;	/* Address of msr (used as hash key) */
	struct hlist_node hlist;
};

static void delete_whitelist(void);
static int create_whitelist(int nentries);
static struct whitelist_entry *find_in_whitelist(u64 msr);
static void add_to_whitelist(struct whitelist_entry *entry);
static u64 get_readmask(loff_t reg);
static u64 get_writemask(loff_t reg);
static int parse_next_whitelist_entry(char *inbuf, char **nextinbuf, 
						struct whitelist_entry *entry);
static ssize_t 
read_whitelist(struct file *file, char __user *buf, size_t count, loff_t *ppos);
static loff_t msr_safe_seek(struct file *file, loff_t offset, int orig);
static ssize_t 
msr_safe_read(struct file *file, char __user *buf, size_t count, loff_t *ppos);
static ssize_t 
msr_safe_write(struct file *file, 
		const char __user *buf, size_t count, loff_t *ppos);
static int msr_safe_open(struct inode *inode, struct file *file);
static int __cpuinit create_msr_safe_device(int cpu);
static void destroy_msr_safe_device(int cpu);
static int __cpuinit 
cdev_class_cpu_callback(struct notifier_block *nfb, 
					unsigned long action, void *hcpu);
static ssize_t version_show(struct kobject *kobj, struct kobj_attribute *attr,
			    char *buf);
static ssize_t version_set(struct kobject *kobj, struct kobj_attribute *attr,
			   const char *buf, size_t count);
static char *msr_safe_nodename(struct device *dev, umode_t *mode);
static void msr_safe_cleanup(void);
static int __init msr_safe_init(void);
static void __exit msr_safe_exit(void);
#endif /* MSR_SAFE_INC */
