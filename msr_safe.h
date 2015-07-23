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
#  Copyright (c) 2011, 2012, 2013, 2014, 2015 by Lawrence Livermore National Security, LLC. LLNL-CODE-645430
#  Produced at the Lawrence Livermore National Laboratory.
#  Written by Marty McFadden, Kathleen Shoga and Barry Rountree (mcfadden1|shoga1|rountree@llnl.gov).
#  All rights reserved.
#
#  This file is part of msr-safe.
#
#  msr-safe is free software: you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation, either
#  version 3 of the License, or (at your option) any
#  later version.
#
#  msr-safe is distributed in the hope that it will be useful, but
#  WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along
#  with msr-safe. If not, see <http://www.gnu.org/licenses/>.
#
#  This material is based upon work supported by the U.S. Department
#  of Energy's Lawrence Livermore National Laboratory. Office of
#  Science, under Award number DE-AC52-07NA27344.
*/
#ifndef MSR_SAFE_INC
#define MSR_SAFE_INC 1
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
#include "msr.h"

#define MSR_NUM_MINORS NR_CPUS

static loff_t msr_safe_seek(struct file *file, loff_t offset, int orig);
static ssize_t msr_safe_read(struct file *file, char __user *buf,
						size_t count, loff_t *ppos);
static ssize_t msr_safe_write(struct file *file,
			const char __user *buf, size_t count, loff_t *ppos);
static int msr_safe_open(struct inode *inode, struct file *file);
static int __cpuinit create_msr_safe_device(int cpu);
static void destroy_msr_safe_device(int cpu);
static int __cpuinit cdev_class_cpu_callback(struct notifier_block *nfb,
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
