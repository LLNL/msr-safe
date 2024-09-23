// Copyright 2011-2020 Lawrence Livermore National Security, LLC and other
// msr-safe Project Developers. See the top-level COPYRIGHT file for
// details.
//
// SPDX-License-Identifier: GPL-2.0-only

// Copyright 2000-2008 H. Peter Anvin - All Rights Reserved
// Copyright 2009 Intel Corporation; author: H. Peter Anvin <hpa@zytor.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * x86 MSR access device
 *
 * This device is accessed by lseek() to the appropriate register number
 * and then read/write in chunks of 8 bytes.  A larger size means multiple
 * reads or writes of the same register.
 *
 * This driver uses /dev/cpu/%d/msr where %d is the minor number, and on
 * an SMP box will direct the access to CPU %d.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <asm/cpufeature.h>
#include <asm/msr.h>
#include <linux/cpu.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/gfp.h>
#include <linux/init.h>
#include <linux/major.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/smp.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/version.h>

#include "msr_batch.h"
#include "msr_allowlist.h"
#include "msr_version.h"

static struct class *msr_class;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0)
static enum cpuhp_state cpuhp_msr_state;
#endif
static int mdev_msr_safe;
static int mdev_msr_allowlist;
static int mdev_msr_batch;
static int mdev_msr_version;

module_param(mdev_msr_safe, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(mdev_msr_safe, "Major number for msr_safe (int).");
module_param(mdev_msr_allowlist, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(mdev_msr_allowlist, "Major number for msr_allowlist (int).");
module_param(mdev_msr_batch, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(mdev_msr_batch, "Major number for msr_batch (int).");
module_param(mdev_msr_version, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(mdev_msr_version, "Major number for msr_version (int).");

static loff_t msr_seek(struct file *file, loff_t offset, int orig)
{
    loff_t ret;
    struct inode *inode = file->f_mapping->host;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,5,0)
    inode_lock(inode);
#else
    mutex_lock(&inode->i_mutex);
#endif
    switch (orig)
    {
        case SEEK_SET:
            file->f_pos = offset;
            ret = file->f_pos;
            break;
        case SEEK_CUR:
            file->f_pos += offset;
            ret = file->f_pos;
            break;
        default:
            ret = -EINVAL;
    }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,5,0)
    inode_unlock(inode);
#else
    mutex_unlock(&inode->i_mutex);
#endif
    return ret;
}

static ssize_t msr_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    u32 __user *tmp = (u32 __user *) buf;
    u32 data[2];
    u32 reg = *ppos;
    int cpu = iminor(file->f_path.dentry->d_inode);
    int err = 0;

    if (count != 8)
    {
        return -EINVAL; /* Single read only.*/
    }

    if (!msr_allowlist_maskexists(reg))
    {
        return -EACCES;
    }

    err = rdmsr_safe_on_cpu(cpu, reg, &data[0], &data[1]);
    if (err)
    {
        return err;
    }

    if (copy_to_user(tmp, &data, 8))
    {
        return -EFAULT;
    }

    return 8;
}

static ssize_t msr_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    const u32 __user *tmp = (const u32 __user *)buf;
    u32 curdata[2];
    u32 data[2];
    u32 reg = *ppos;
    u64 mask;
    int cpu = iminor(file->f_path.dentry->d_inode);
    int err = 0;

    if (count != 8) /* single write only */
    {
        return -EINVAL; 
    }

    mask = msr_allowlist_writemask(reg);

    if (mask == 0)
    {
        return -EACCES;
    }

    if (copy_from_user(&data, tmp, 8))
    {
        return -EFAULT;
    }

    if (mask != 0xffffffffffffffff)
    {
        err = rdmsr_safe_on_cpu(cpu, reg, &curdata[0], &curdata[1]);
        if (err)
        {
            return err;
        }

        *(u64 *)&curdata[0] &= ~mask;
        *(u64 *)&data[0] &= mask;
        *(u64 *)&data[0] |= *(u64 *)&curdata[0];
    }

    err = wrmsr_safe_on_cpu(cpu, reg, data[0], data[1]);
    if (err)
    {
        return err;
    }

    return 8;
}

static int msr_open(struct inode *inode, struct file *file)
{
    unsigned int cpu = iminor(file->f_path.dentry->d_inode);

    if (cpu >= nr_cpu_ids || !cpu_online(cpu))
    {
        return -ENXIO;  // No such CPU
    }

    return 0;
}

static int msr_close(struct inode *inode, struct file *file)
{
    return 0;
}

/* File operations we support */
static const struct file_operations msr_fops =
{
    .owner = THIS_MODULE,
    .llseek = msr_seek,
    .read = msr_read,
    .write = msr_write,
    .open = msr_open,
    .release = msr_close
};

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,10,0)
static int msr_device_create(int cpu)
#else
static int msr_device_create(unsigned int cpu)
#endif
{
    struct device *dev;

    dev = device_create(msr_class, NULL, MKDEV(mdev_msr_safe, cpu), NULL, "msr_safe%d", cpu);
    return IS_ERR(dev) ? PTR_ERR(dev) : 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,10,0)
static void msr_device_destroy(int cpu)
{
    device_destroy(msr_class, MKDEV(mdev_msr_safe, cpu));
}
#else
static int msr_device_destroy(unsigned int cpu)
{
    device_destroy(msr_class, MKDEV(mdev_msr_safe, cpu));
    return 0;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,10,0)
static int msr_class_cpu_callback(struct notifier_block *nfb, unsigned long action, void *hcpu)
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

static struct notifier_block __refdata msr_class_cpu_notifier =
{
    .notifier_call = msr_class_cpu_callback,
};
#endif

/* The type of (((struct class *)0)->devnode) has changed at least
 * twice in the mainline linux kernel.  Trying to pin these changes
 * to specific mainline kernel versions runs into the problem of
 * distros backporting features to older kernels.  Instead, we are
 * now keying off of the change in the parameter types.*/

#define msr_devnode_selector _Generic(\
    (((struct class *)0)->devnode),\
    char * (*) (struct device *, mode_t *) : msr_devnode1,\
    char * (*) (struct device *, umode_t *) : msr_devnode2,\
    char * (*) (const struct device *, umode_t *) : msr_devnode3\
    )

static char *msr_devnode1(struct device *dev, mode_t *mode)
{
    return kasprintf(GFP_KERNEL, "cpu/%u/msr_safe", MINOR(dev->devt));
}

static char *msr_devnode2(struct device *dev, umode_t *mode)
{
    return kasprintf(GFP_KERNEL, "cpu/%u/msr_safe", MINOR(dev->devt));
}

static char *msr_devnode3(const struct device *dev, umode_t *mode)
{
    return kasprintf(GFP_KERNEL, "cpu/%u/msr_safe", MINOR(dev->devt));
}

static int __init msr_init(void)
{
    int err = 0;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,10,0)
    int i;
#endif

    err = msrbatch_init(&mdev_msr_batch);
    if (err != 0)
    {
        pr_debug("failed to initialize msr_batch\n");
        goto out;
    }

    err = msr_allowlist_init(&mdev_msr_allowlist);
    if (err != 0)
    {
        pr_debug("failed to initialize msr_allowlist\n");
        goto out_batch;
    }

    err = msr_version_init(&mdev_msr_version);
    if (err != 0)
    {
        pr_debug("failed to initialize msr_version\n");
        goto out_version;
    }

    /*
     * register_chrdev will return:
     *    If major == 0, dynamically allocate a major and return its number
     *    If major > 0,  attempt to reserve a device with the given major
     *      number and return zero on success
     *    Return a negative errno on failure
     */
    err = __register_chrdev(mdev_msr_safe, 0, num_possible_cpus(), "cpu/msr_safe", &msr_fops);
    if (err < 0)
    {
        pr_debug("unable to get major %d for msr_safe\n", mdev_msr_safe);
        err = -EBUSY;
        goto out_wlist;
    }
    if (err > 0)
    {
        mdev_msr_safe = err;
    }

    pr_debug("msr_safe major dev: %i\n", mdev_msr_safe);
    pr_debug("msr_batch major dev: %i\n", mdev_msr_batch);
    pr_debug("msr_allowlist major dev: %i\n", mdev_msr_allowlist);
    pr_debug("msr_version major dev: %i\n", mdev_msr_version);

    msr_class = class_create(
#if LINUX_VERSION_CODE < KERNEL_VERSION(6,4,0)
                    THIS_MODULE,
#endif
                    "msr_safe");
    if (IS_ERR(msr_class))
    {
        err = PTR_ERR(msr_class);
        goto out_chrdev;
    }
    msr_class->devnode = msr_devnode_selector;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,10,0)
    i = 0;
    for_each_online_cpu(i)
    err = msr_device_create(i);
    if (err != 0)
    {
        goto out_class;
    }
    register_hotcpu_notifier(&msr_class_cpu_notifier);
#else
    err = cpuhp_setup_state(CPUHP_AP_ONLINE_DYN, "x86/msr:online", msr_device_create, msr_device_destroy);
    if (err < 0)
    {
        goto out_class;
    }
    cpuhp_msr_state = err;
#endif
    return 0;

out_class:
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,10,0)
    i = 0;
    for_each_online_cpu(i)
    msr_device_destroy(i);
#endif
    class_destroy(msr_class);
out_chrdev:
    __unregister_chrdev(mdev_msr_safe, 0, num_possible_cpus(), "cpu/msr_safe");
out_wlist:
    msr_allowlist_cleanup(mdev_msr_allowlist);
out_batch:
    msrbatch_cleanup(mdev_msr_batch);
out_version:
    msr_version_cleanup(mdev_msr_version);
out:
    return err;
}
module_init(msr_init);

static void __exit msr_exit(void)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,10,0)
    int cpu = 0;

    for_each_online_cpu(cpu)
    msr_device_destroy(cpu);
#else
    cpuhp_remove_state(cpuhp_msr_state);
#endif
    class_destroy(msr_class);
    __unregister_chrdev(mdev_msr_safe, 0, num_possible_cpus(), "cpu/msr_safe");

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,10,0)
    unregister_hotcpu_notifier(&msr_class_cpu_notifier);
#endif

    msr_allowlist_cleanup(mdev_msr_allowlist);
    msrbatch_cleanup(mdev_msr_batch);
    msr_version_cleanup(mdev_msr_version);
}

module_exit(msr_exit)

MODULE_AUTHOR("M. Fadden, K. Shoga, B. Rountree, H. P. Anvin");
MODULE_DESCRIPTION("x86 generic MSR driver (+LLNL Approved List)");
MODULE_VERSION(MSR_SAFE_VERSION_STR);
MODULE_LICENSE("GPL");
