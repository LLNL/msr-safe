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
#include "msr_whitelist.h"

static struct class *msr_class;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0)
static enum cpuhp_state cpuhp_msr_state;
#endif
static int mdev_msr_safe;
static int mdev_msr_whitelist;
static int mdev_msr_batch;

module_param(mdev_msr_safe, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(mdev_msr_safe, "Major number for msr_safe (int).");
module_param(mdev_msr_whitelist, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(mdev_msr_whitelist, "Major number for msr_whitelist (int).");
module_param(mdev_msr_batch, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(mdev_msr_batch, "Major number for msr_batch (int).");

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
    ssize_t bytes = 0;

    if (count % 8)
    {
        return -EINVAL; /* Invalid chunk size */
    }

    if (!capable(CAP_SYS_RAWIO) && !msr_whitelist_maskexists(reg))
    {
        return -EACCES;
    }

    for (; count; count -= 8)
    {
        err = rdmsr_safe_on_cpu(cpu, reg, &data[0], &data[1]);
        if (err)
        {
            break;
        }
        if (copy_to_user(tmp, &data, 8))
        {
            err = -EFAULT;
            break;
        }
        tmp += 2;
        bytes += 8;
    }

    return bytes ? bytes : err;
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
    ssize_t bytes = 0;

    if (count % 8)
    {
        return -EINVAL; // Invalid chunk size
    }

    mask = capable(CAP_SYS_RAWIO) ? 0xffffffffffffffff : msr_whitelist_writemask(reg);

    if (!capable(CAP_SYS_RAWIO) && mask == 0)
    {
        return -EACCES;
    }

    for (; count; count -= 8)
    {
        if (copy_from_user(&data, tmp, 8))
        {
            err = -EFAULT;
            break;
        }

        if (mask != 0xffffffffffffffff)
        {
            err = rdmsr_safe_on_cpu(cpu, reg, &curdata[0], &curdata[1]);
            if (err)
            {
                break;
            }

            *(u64 *)&curdata[0] &= ~mask;
            *(u64 *)&data[0] &= mask;
            *(u64 *)&data[0] |= *(u64 *)&curdata[0];
        }

        err = wrmsr_safe_on_cpu(cpu, reg, data[0], data[1]);
        if (err)
        {
            break;
        }
        tmp += 2;
        bytes += 8;
    }

    return bytes ? bytes : err;
}

static long msr_ioctl(struct file *file, unsigned int ioc, unsigned long arg)
{
    u32 __user *uregs = (u32 __user *)arg;
    u32 regs[8];
    int cpu = iminor(file->f_path.dentry->d_inode);
    int err = 0;

    if (!capable(CAP_SYS_RAWIO))
    {
        return -EACCES;
    }

    switch (ioc)
    {
        case X86_IOC_RDMSR_REGS:
            if (!(file->f_mode & FMODE_READ))
            {
                err = -EBADF;
                break;
            }
            if (copy_from_user(&regs, uregs, sizeof(regs)))
            {
                err = -EFAULT;
                break;
            }
            err = rdmsr_safe_regs_on_cpu(cpu, regs);
            if (err)
            {
                break;
            }
            if (copy_to_user(uregs, &regs, sizeof(regs)))
            {
                err = -EFAULT;
            }
            break;
        case X86_IOC_WRMSR_REGS:
            if (!(file->f_mode & FMODE_WRITE))
            {
                err = -EBADF;
                break;
            }
            if (copy_from_user(&regs, uregs, sizeof(regs)))
            {
                err = -EFAULT;
                break;
            }
            err = wrmsr_safe_regs_on_cpu(cpu, regs);
            if (err)
            {
                break;
            }
            if (copy_to_user(uregs, &regs, sizeof(regs)))
            {
                err = -EFAULT;
            }
            break;
        default:
            err = -ENOTTY;
            break;
    }

    return err;
}

static int msr_open(struct inode *inode, struct file *file)
{
    unsigned int cpu = iminor(file->f_path.dentry->d_inode);
    struct cpuinfo_x86 *c;

    if (cpu >= nr_cpu_ids || !cpu_online(cpu))
    {
        return -ENXIO;  // No such CPU
    }

    c = &cpu_data(cpu);
    if (!cpu_has(c, X86_FEATURE_MSR))
    {
        return -EIO; // MSR not supported
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
    .unlocked_ioctl = msr_ioctl,
    .compat_ioctl = msr_ioctl,
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

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,39)
static char *msr_devnode(struct device *dev, mode_t *mode)
#else
static char *msr_devnode(struct device *dev, umode_t *mode)
#endif
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

    err = msr_whitelist_init(&mdev_msr_whitelist);
    if (err != 0)
    {
        pr_debug("failed to initialize msr_whitelist\n");
        goto out_batch;
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
    pr_debug("msr_whitelist major dev: %i\n", mdev_msr_whitelist);

    msr_class = class_create(THIS_MODULE, "msr_safe");
    if (IS_ERR(msr_class))
    {
        err = PTR_ERR(msr_class);
        goto out_chrdev;
    }
    msr_class->devnode = msr_devnode;

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
    msr_whitelist_cleanup(mdev_msr_whitelist);
out_batch:
    msrbatch_cleanup(mdev_msr_batch);
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

    msr_whitelist_cleanup(mdev_msr_whitelist);
    msrbatch_cleanup(mdev_msr_batch);
}

module_exit(msr_exit)

MODULE_AUTHOR("H. Peter Anvin <hpa@zytor.com>");
MODULE_DESCRIPTION("x86 generic MSR driver (+LLNL Whitelist)");
MODULE_VERSION("1.2");
MODULE_LICENSE("GPL");
