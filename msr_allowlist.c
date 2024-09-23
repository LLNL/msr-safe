// Copyright 2011-2021 Lawrence Livermore National Security, LLC and other
// msr-safe Project Developers. See the top-level COPYRIGHT file for
// details.
//
// SPDX-License-Identifier: GPL-2.0-only

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

#include "msr_allowlist.h"

#define MAX_WLIST_BSIZE ((128 * 1024) + 1) // "+1" for null character

struct allowlist_entry
{
    u64 wmask; // Bits that may be written
    u64 msr;   // Address of msr (used as hash key)
    struct hlist_node hlist;
};

struct allowlist_data
{
    u32 entries;     // WL entries successfully parsed
    int ecode;       // 0 if things look good
    char *partial_line;
    int partial_len; // Length of partial line
    struct allowlist_entry *allowlist;
};

static void delete_allowlist(void);
static int create_allowlist(int nentries);
static struct allowlist_entry *find_in_allowlist(u64 msr);
static void add_to_allowlist(struct allowlist_entry *entry);
static int parse_next_allowlist_entry(char *inbuf, char **nextinbuf, struct allowlist_entry *entry);
static ssize_t read_allowlist(struct file *file, char __user *buf, size_t count, loff_t *ppos);
static struct class *cdev_class;
static char cdev_created;
static char cdev_registered;
static char cdev_class_created;

static DEFINE_HASHTABLE(allowlist_hash, 6);
static DEFINE_MUTEX(allowlist_mutex);
static struct allowlist_entry *allowlist;
static int allowlist_numentries;

int msr_allowlist_maskexists(loff_t reg)
{
    struct allowlist_entry *entry;

    mutex_lock(&allowlist_mutex);
    entry = find_in_allowlist((u64)reg);
    mutex_unlock(&allowlist_mutex);

    return entry != NULL;
}

u64 msr_allowlist_writemask(loff_t reg)
{
    struct allowlist_entry *entry;

    mutex_lock(&allowlist_mutex);
    entry = find_in_allowlist((u64)reg);
    mutex_unlock(&allowlist_mutex);

    return entry ? entry->wmask : 0;
}

static int open_allowlist(struct inode *inode, struct file *file)
{
    return 0;
}

/*
 * After copying data from user space, we make two passes through it.
 * The first pass is to ensure that the input file is valid. If the file is
 * valid, we will then delete the current white list and then perform the
 * second pass to actually create the new white list.
 */
static ssize_t write_allowlist(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    int err = 0;
    const u32 __user *tmp = (const u32 __user *)buf;
    char *s;
    int res;
    int num_entries;
    struct allowlist_entry *entry;
    char *kbuf;

    pr_debug("%s: %p, %zu, %llu\n", __FUNCTION__, buf, count, *ppos);
    if (count <= 2)
    {
        mutex_lock(&allowlist_mutex);
        delete_allowlist();
        hash_init(allowlist_hash);
        mutex_unlock(&allowlist_mutex);
        return count;
    }

    if (count+1 > MAX_WLIST_BSIZE)
    {
        pr_err("%s: buffer of %zu bytes too large\n", __FUNCTION__, count);
        return -E2BIG;
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
        res = parse_next_allowlist_entry(s, &s, 0);
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
        err = -ENOMSG;
        goto out_freebuffer;
    }

    /* Pass 2: */
    mutex_lock(&allowlist_mutex);
    res = create_allowlist(num_entries);
    if (res < 0)
    {
        pr_err("%s: create_allowlist(%d) failed\n", __FUNCTION__, num_entries);
        err = res;
        goto out_releasemutex;
    }

    for (entry = allowlist, s = kbuf, res = 1; res > 0; entry++)
    {
        res = parse_next_allowlist_entry(s, &s, entry);
        if (res < 0)
        {
            pr_err("%s: Table corrupted\n", __FUNCTION__);
            delete_allowlist();
            err = res; // This should not happen!
            goto out_releasemutex;
        }

        if (res)
        {
            if (find_in_allowlist(entry->msr))
            {
                pr_err("%s: Duplicate: %llX\n", __FUNCTION__, entry->msr);
                err = -ENOTUNIQ;
                delete_allowlist();
                goto out_releasemutex;
            }
            add_to_allowlist(entry);
        }
    }

out_releasemutex:
    mutex_unlock(&allowlist_mutex);
out_freebuffer:
    kfree(kbuf);
    return err ? err : count;
}

static ssize_t read_allowlist(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    loff_t idx = *ppos;
    u32 __user *tmp = (u32 __user *) buf;
    char kbuf[200];
    int len;
    struct allowlist_entry e;

    mutex_lock(&allowlist_mutex);
    *ppos = 0;

    if (idx >= allowlist_numentries || idx < 0)
    {
        mutex_unlock(&allowlist_mutex);
        return 0;
    }

    e = allowlist[idx];
    mutex_unlock(&allowlist_mutex);

    if (idx == 0)
    {
        len = sprintf(kbuf, "%-10s %-18s\n" "0x%08llX 0x%016llX\n", "#MSR", "Write mask", e.msr, e.wmask);
    }
    else
    {
        len = sprintf(kbuf, "0x%08llX 0x%016llX\n", e.msr, e.wmask);
    }


    if (len > count)
    {
        return -E2BIG;
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
    .read = read_allowlist,
    .write = write_allowlist,
    .open = open_allowlist
};

static void delete_allowlist(void)
{
    if (ZERO_OR_NULL_PTR(allowlist))
    {
        return;
    }

    kfree(allowlist);
    allowlist = 0;
    allowlist_numentries = 0;
}

static int create_allowlist(int nentries)
{
    hash_init(allowlist_hash);
    delete_allowlist();
    allowlist_numentries = nentries;
    allowlist = kcalloc(nentries, sizeof(*allowlist), GFP_KERNEL);

    if (ZERO_OR_NULL_PTR(allowlist))
    {
        return -ENOMEM;
    }
    return 0;
}

static struct allowlist_entry *find_in_allowlist(u64 msr)
{
    struct allowlist_entry *entry = 0;

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,39)
    struct hlist_node *node;
    if (allowlist)
    {
        hash_for_each_possible(allowlist_hash, entry, node, hlist, msr)
        if (entry && entry->msr == msr)
        {
            return entry;
        }
    }
#else
    if (allowlist)
    {
        hash_for_each_possible(allowlist_hash, entry, hlist, msr)
        if (entry && entry->msr == msr)
        {
            return entry;
        }
    }
#endif
    return 0;
}


static void add_to_allowlist(struct allowlist_entry *entry)
{
    hash_add(allowlist_hash, &entry->hlist, entry->msr);
}

static int parse_next_allowlist_entry(char *inbuf, char **nextinbuf, struct allowlist_entry *entry)
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
            return -EILSEQ;
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


#define msr_allowlist_nodename_selector _Generic(\
    (((struct class *)0)->devnode),\
    char* (*) (struct device *,       mode_t  *) : msr_allowlist_nodename1,\
    char* (*) (struct device *,       umode_t *) : msr_allowlist_nodename2,\
    char* (*) (const struct device *, umode_t *) : msr_allowlist_nodename3 \
    )

static char *msr_allowlist_nodename1(struct device *dev, mode_t *mode)
{
    return kasprintf(GFP_KERNEL, "cpu/msr_allowlist");
}

static char *msr_allowlist_nodename2(struct device *dev, umode_t *mode)
{
    return kasprintf(GFP_KERNEL, "cpu/msr_allowlist");
}

static char *msr_allowlist_nodename3(const struct device *dev, umode_t *mode)
{
    return kasprintf(GFP_KERNEL, "cpu/msr_allowlist");
}

void msr_allowlist_cleanup(int majordev)
{
    delete_allowlist();

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
        unregister_chrdev(majordev, "cpu/msr_allowlist");
    }
}

int msr_allowlist_init(int *majordev)
{
    int err = 0;
    struct device *dev;

    err = register_chrdev(*majordev, "cpu/msr_allowlist", &fops);
    if (err < 0)
    {
        pr_debug("%s: unable to register chrdev\n", __FUNCTION__);
        msr_allowlist_cleanup(*majordev);
        err = -EBUSY;
        return err;
    }
    if (err > 0)
    {
        *majordev = err;
    }
    cdev_registered = 1;

    cdev_class = class_create(
#if LINUX_VERSION_CODE < KERNEL_VERSION(6,4,0)
                    THIS_MODULE,
#endif
                    "msr_allowlist");
    if (IS_ERR(cdev_class))
    {
        err = PTR_ERR(cdev_class);
        msr_allowlist_cleanup(*majordev);
        return err;
    }
    cdev_class_created = 1;

    cdev_class->devnode = msr_allowlist_nodename_selector;

    dev = device_create(cdev_class, NULL, MKDEV(*majordev, 0), NULL, "msr_allowlist");
    if (IS_ERR(dev))
    {
        err = PTR_ERR(dev);
        msr_allowlist_cleanup(*majordev);
        return err;
    }
    cdev_created = 1;
    return 0;
}
