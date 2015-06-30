#ifndef MSR_WHITELIST_IMPL_INC
#define MSR_WHITELIST__IMPL_INC 1
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
 * implementation specific definitions
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

#define MAX_WLIST_BSIZE ((128 * 1024) + 1) /* "+1" for null character */

/*
 * If the msr is writeable and the flag is 0, the msr is erased when the 
 * whitelist is removed.  If the flag is 1 then the value for the msr is 
 * recorded somewhere and restored when the whitelist is removed
 */
#define CLEAR_MSR_ON_WHITELIST_REMOVE(x)	((x) == 1)
#define RESTORE_MSR_ON_WHITELIST_REMOVE(x)	((x) == 2)
struct whitelist_entry {
	u64 wmask;	/* Bits that may be written */
	u64 rmask;	/* Bits that may be read */
	u64 msr;	/* Address of msr (used as hash key) */
	int resflag;	/* 0=do-nothing, 1=erase when removed, 2=restore */
	u64 *msrdata;	/* ptr to original msr contents of writable bits */
	struct hlist_node hlist;
};

static void delete_whitelist(void);
static int create_whitelist(int nentries);
static struct whitelist_entry *find_in_whitelist(u64 msr);
static void add_to_whitelist(struct whitelist_entry *entry);
static int parse_next_whitelist_entry(char *inbuf, char **nextinbuf, 
						struct whitelist_entry *entry);
static ssize_t read_whitelist(struct file *file, char __user *buf, 
						size_t count, loff_t *ppos);
#endif /* MSR_WHITELIST_IMPL_INC */
