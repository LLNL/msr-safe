/*
 * Copyright (c) 2011-2017 by Lawrence Livermore National Security, LLC.
 * LLNL-CODE-645430
 *
 * Produced at Lawrence Livermore National Laboratory.
 * Written by  Marty McFadden, mcfadden8@llnl.gov
 *             Kathleen Shoga, shoga1@llnl.gov
 *             Barry Rountree, rountree@llnl.gov
 *
 * All rights reserved.
 *
 * This file is part of msr-safe.
 *
 * msr-safe is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * msr-safe is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with msr-safe. If not, see <http://www.gnu.org/licenses/>.
 *
 * This material is based upon work supported by the U.S. Department of
 * Energy's Lawrence Livermore National Laboratory. Office of Science, under
 * Award number DE-AC52-07NA27344.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/ctype.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/hashtable.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/version.h>

#define MAX_WLIST_BSIZE ((128 * 1024) + 1) // "+1" for null character

struct approved_list_entry
{
    u64 wmask; // Bits that may be written
    u64 msr;   // Address of msr (used as hash key)
    struct hlist_node hlist;
};

struct approved_list_data
{
    u32 entries;     // WL entries successfully parsed
    int ecode;       // 0 if things look good
    char *partial_line;
    int partial_len; // Length of partial line
    struct approved_list_entry *approved_list;
};

static void delete_approved_list(void);
static int create_approved_list(int nentries);
static struct approved_list_entry *find_in_approved_list(u64 msr);
static void add_to_approved_list(struct approved_list_entry *entry);
static int parse_next_approved_list_entry(char *inbuf, char **nextinbuf, struct approved_list_entry *entry);
static ssize_t read_approved_list(struct file *file, char __user *buf, size_t count, loff_t *ppos);
static struct class *cdev_class;
static char cdev_created;
static char cdev_registered;
static char cdev_class_created;

static DEFINE_HASHTABLE(approved_list_hash, 6);
static DEFINE_MUTEX(approved_list_mutex);
static struct approved_list_entry *approved_list;
static int approved_list_numentries;

int msr_approved_list_maskexists(loff_t reg)
{
    struct approved_list_entry *entry;

    mutex_lock(&approved_list_mutex);
    entry = find_in_approved_list((u64)reg);
    mutex_unlock(&approved_list_mutex);

    return entry != NULL;
}

u64 msr_approved_list_writemask(loff_t reg)
{
    struct approved_list_entry *entry;

    mutex_lock(&approved_list_mutex);
    entry = find_in_approved_list((u64)reg);
    mutex_unlock(&approved_list_mutex);

    return entry ? entry->wmask : 0;
}

static int open_approved_list(struct inode *inode, struct file *file)
{
    return 0;
}

/*
 * After copying data from user space, we make two passes through it.
 * The first pass is to ensure that the input file is valid. If the file is
 * valid, we will then delete the current white list and then perform the
 * second pass to actually create the new white list.
 */
static ssize_t write_approved_list(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    int err = 0;
    const u32 __user *tmp = (const u32 __user *)buf;
    char *s;
    int res;
    int num_entries;
    struct approved_list_entry *entry;
    char *kbuf;

    pr_debug("%s: %p, %zu, %llu\n", __FUNCTION__, buf, count, *ppos);
    if (count <= 2)
    {
        mutex_lock(&approved_list_mutex);
        delete_approved_list();
        hash_init(approved_list_hash);
        mutex_unlock(&approved_list_mutex);
        return count;
    }

    if (count+1 > MAX_WLIST_BSIZE)
    {
        pr_err("%s: buffer of %zu bytes too large\n", __FUNCTION__, count);
        return -EINVAL;
    }

    kbuf = kzalloc(count+1, GFP_KERNEL);
    if (ZERO_OR_NULL_PTR(kbuf))
    {
        pr_err("%s: memory alloc(%zu) failed\n", __FUNCTION__, count+1);
        return -ENOMEM;
    }

    if (copy_from_user(kbuf, tmp, count))
    {
        pr_err("%s: copy_from_user(%zu) failed\n", __FUNCTION__, count);
        err = -EFAULT;
        goto out_freebuffer;
    }

    /* Pass 1: */
    for (num_entries = 0, s = kbuf, res = 1; res > 0; )
    {
        res = parse_next_approved_list_entry(s, &s, 0);
        if (res < 0)
        {
            pr_err("%s: parse error\n", __FUNCTION__);
            err = res;
            goto out_freebuffer;
        }

        if (res)
        {
            num_entries++;
        }
    }

    if (num_entries == 0)
    {
        pr_err("%s: No valid entries found in %zu bytes of input\n", __FUNCTION__, count);
        err = -EINVAL;
        goto out_freebuffer;
    }

    /* Pass 2: */
    mutex_lock(&approved_list_mutex);
    res = create_approved_list(num_entries);
    if (res < 0)
    {
        pr_err("%s: create_approved_list(%d) failed\n", __FUNCTION__, num_entries);
        err = res;
        goto out_releasemutex;
    }

    for (entry = approved_list, s = kbuf, res = 1; res > 0; entry++)
    {
        res = parse_next_approved_list_entry(s, &s, entry);
        if (res < 0)
        {
            pr_err("%s: Table corrupted\n", __FUNCTION__);
            delete_approved_list();
            err = res; // This should not happen!
            goto out_releasemutex;
        }

        if (res)
        {
            if (find_in_approved_list(entry->msr))
            {
                pr_err("%s: Duplicate: %llX\n", __FUNCTION__, entry->msr);
                err = -EINVAL;
                delete_approved_list();
                goto out_releasemutex;
            }
            add_to_approved_list(entry);
        }
    }

out_releasemutex:
    mutex_unlock(&approved_list_mutex);
out_freebuffer:
    kfree(kbuf);
    return err ? err : count;
}

static ssize_t read_approved_list(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    loff_t idx = *ppos;
    u32 __user *tmp = (u32 __user *) buf;
    char kbuf[200];
    int len;
    struct approved_list_entry e;

    mutex_lock(&approved_list_mutex);
    *ppos = 0;

    if (idx >= approved_list_numentries || idx < 0)
    {
        mutex_unlock(&approved_list_mutex);
        return 0;
    }

    e = approved_list[idx];
    mutex_unlock(&approved_list_mutex);

    if (idx == 0)
    {
        len = sprintf(kbuf, "# MSR # Write Mask # Comment\n" "0x%08llX 0x%016llX\n", e.msr, e.wmask);
    }
    else
    {
        len = sprintf(kbuf, "0x%08llX 0x%016llX\n", e.msr, e.wmask);
    }


    if (len > count)
    {
        return -EFAULT;
    }

    if (copy_to_user(tmp, kbuf, len))
    {
        return -EFAULT;
    }

    *ppos = idx+1;
    return len;
}

static const struct file_operations fops =
{
    .owner = THIS_MODULE,
    .read = read_approved_list,
    .write = write_approved_list,
    .open = open_approved_list
};

static void delete_approved_list(void)
{
    if (ZERO_OR_NULL_PTR(approved_list))
    {
        return;
    }

    kfree(approved_list);
    approved_list = 0;
    approved_list_numentries = 0;
}

static int create_approved_list(int nentries)
{
    hash_init(approved_list_hash);
    delete_approved_list();
    approved_list_numentries = nentries;
    approved_list = kcalloc(nentries, sizeof(*approved_list), GFP_KERNEL);

    if (ZERO_OR_NULL_PTR(approved_list))
    {
        return -ENOMEM;
    }
    return 0;
}

static struct approved_list_entry *find_in_approved_list(u64 msr)
{
    struct approved_list_entry *entry = 0;

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,39)
    struct hlist_node *node;
    if (approved_list)
    {
        hash_for_each_possible(approved_list_hash, entry, node, hlist, msr)
        if (entry && entry->msr == msr)
        {
            return entry;
        }
    }
#else
    if (approved_list)
    {
        hash_for_each_possible(approved_list_hash, entry, hlist, msr)
        if (entry && entry->msr == msr)
        {
            return entry;
        }
    }
#endif
    return 0;
}


static void add_to_approved_list(struct approved_list_entry *entry)
{
    hash_add(approved_list_hash, &entry->hlist, entry->msr);
}

static int parse_next_approved_list_entry(char *inbuf, char **nextinbuf, struct approved_list_entry *entry)
{
    char *s = skip_spaces(inbuf);
    int i;
    u64 data[2];

    while (*s == '#')
    {
        /* Skip remaining portion of line */
        for (s = s + 1; *s && *s != '\n'; s++)
        {
            ;
        }
        s = skip_spaces(s);
    }

    if (*s == 0)
    {
        return 0; // This means we are done with the input buffer
    }

    for (i = 0; i < 2; i++)
    {
        /* we should have the first of 3 #s now */
        char *s2;
        int err;
        char tmp;

        s2 = s = skip_spaces(s);
        while (!isspace(*s) && *s)
        {
            s++;
        }

        if (*s == 0)
        {
            pr_debug("%s: Premature EOF\n", __FUNCTION__);
            return -EINVAL;
        }

        tmp = *s;
        *s = 0; /* Null-terminate this portion of string */
        err = kstrtoull(s2, 0, &data[i]);
        if (err)
        {
            return err;
        }
        *s++ = tmp;
    }

    if (entry)
    {
        entry->msr = data[0];
        entry->wmask = data[1];
    }

    *nextinbuf = s; /* Return where we left off to caller */
    return *nextinbuf - inbuf;
}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,39)
static char *msr_approved_list_nodename(struct device *dev, mode_t *mode)
#else
static char *msr_approved_list_nodename(struct device *dev, umode_t *mode)
#endif
{
    return kasprintf(GFP_KERNEL, "cpu/msr_approved_list");
}

void msr_approved_list_cleanup(int majordev)
{
    delete_approved_list();

    if (cdev_created)
    {
        cdev_created = 0;
        device_destroy(cdev_class, MKDEV(majordev, 0));
    }

    if (cdev_class_created)
    {
        cdev_class_created = 0;
        class_destroy(cdev_class);
    }

    if (cdev_registered)
    {
        cdev_registered = 0;
        unregister_chrdev(majordev, "cpu/msr_approved_list");
    }
}

int msr_approved_list_init(int *majordev)
{
    int err = 0;
    struct device *dev;

    err = register_chrdev(*majordev, "cpu/msr_approved_list", &fops);
    if (err < 0)
    {
        pr_debug("%s: unable to register chrdev\n", __FUNCTION__);
        msr_approved_list_cleanup(*majordev);
        err = -EBUSY;
        return err;
    }
    if (err > 0)
    {
        *majordev = err;
    }
    cdev_registered = 1;

    cdev_class = class_create(THIS_MODULE, "msr_approved_list");
    if (IS_ERR(cdev_class))
    {
        err = PTR_ERR(cdev_class);
        msr_approved_list_cleanup(*majordev);
        return err;
    }
    cdev_class_created = 1;

    cdev_class->devnode = msr_approved_list_nodename;

    dev = device_create(cdev_class, NULL, MKDEV(*majordev, 0), NULL, "msr_approved_list");
    if (IS_ERR(dev))
    {
        err = PTR_ERR(dev);
        msr_approved_list_cleanup(*majordev);
        return err;
    }
    cdev_created = 1;
    return 0;
}
