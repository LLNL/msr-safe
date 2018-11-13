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

/*
 * x86 MSR batch access device
 *
 * This device is accessed by ioctl() to submit a batch of MSR requests
 * which may be used instead of or in addition to the lseek()/write()/read()
 * mechanism provided by msr_safe.c
 *
 * This driver uses /dev/cpu/msr_batch as its device file.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <asm/msr.h>
#include <linux/cpu.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/uaccess.h>
#include <linux/version.h>

#include "msr_batch.h"
#include "msr_safe.h"
#include "msr_whitelist.h"

static struct class *cdev_class;
static char cdev_created;
static char cdev_registered;
static char cdev_class_created;

static int msrbatch_open(struct inode *inode, struct file *file)
{
    unsigned int cpu;
    struct cpuinfo_x86 *c;

    cpu = iminor(file->f_path.dentry->d_inode);
    if (cpu >= nr_cpu_ids || !cpu_online(cpu))
    {
        pr_debug("cpu #%u does not exist\n", cpu);
        return -ENXIO; // No such CPU
    }

    c = &cpu_data(cpu);
    if (!cpu_has(c, X86_FEATURE_MSR))
    {
        pr_debug("cpu #%u does not have MSR feature.\n", cpu);
        return -EIO; // MSR not supported
    }

    return 0;
}

static int msrbatch_close(struct inode *inode, struct file *file)
{
    return 0;
}

static int msrbatch_apply_whitelist(struct msr_batch_array *oa)
{
    struct msr_batch_op *op;
    int err = 0;
    bool has_sys_rawio_cap = capable(CAP_SYS_RAWIO);

    for (op = oa->ops; op < oa->ops + oa->numops; ++op)
    {
        op->err = 0;

        if (op->cpu >= nr_cpu_ids || !cpu_online(op->cpu))
        {
            pr_debug("No such CPU %d\n", op->cpu);
            op->err = err = -ENXIO; // No such CPU
            continue;
        }

        if (has_sys_rawio_cap)
        {
            op->wmask = 0xffffffffffffffff;
            continue;
        }

        if (!msr_whitelist_maskexists(op->msr))
        {
            pr_debug("No whitelist entry for MSR %x\n", op->msr);
            op->err = err = -EACCES;
        }
        else
        {
            op->wmask = msr_whitelist_writemask(op->msr);
            /* Check for read-only case */
            if (op->wmask == 0 && !op->isrdmsr)
            {
                pr_debug("MSR %x is read-only\n", op->msr);
                op->err = err = -EACCES;
            }
        }
    }
    return err;
}

extern int msr_safe_batch(struct msr_batch_array *oa);

static long msrbatch_ioctl(struct file *f, unsigned int ioc, unsigned long arg)
{
    int err = 0;
    struct msr_batch_array __user *uoa;
    struct msr_batch_op __user *uops;
    struct msr_batch_array koa;

    if (ioc != X86_IOC_MSR_BATCH)
    {
        pr_debug("Invalid ioctl op %u\n", ioc);
        return -ENOTTY;
    }

    if (!(f->f_mode & FMODE_READ))
    {
        pr_debug("File not open for reading\n");
        return -EBADF;
    }

    uoa = (struct msr_batch_array *)arg;

    if (copy_from_user(&koa, uoa, sizeof(koa)))
    {
        pr_debug("Copy of batch array descriptor failed\n");
        return -EFAULT;
    }

    if (koa.numops <= 0)
    {
        pr_debug("Invalid # of ops %d\n", koa.numops);
        return -EINVAL;
    }

    uops = koa.ops;

    koa.ops = kmalloc_array(koa.numops, sizeof(*koa.ops), GFP_KERNEL);
    if (!koa.ops)
    {
        return -ENOMEM;
    }

    if (copy_from_user(koa.ops, uops, koa.numops * sizeof(*koa.ops)))
    {
        pr_debug("Copy of batch array failed\n");
        err = -EFAULT;
        goto bundle_alloc;
    }

    err = msrbatch_apply_whitelist(&koa);
    if (err)
    {
        pr_debug("Failed to apply whitelist %d\n", err);
        goto copyout_and_return;
    }

    err = msr_safe_batch(&koa);
    if (err != 0)
    {
        pr_debug("msr_safe_batch failed: %d\n", err);
        goto copyout_and_return;
    }

copyout_and_return:
    if (copy_to_user(uops, koa.ops, koa.numops * sizeof(*uops)))
    {
        pr_debug("copy batch data back to user failed\n");
        if (!err)
        {
            err = -EFAULT;
        }
    }
bundle_alloc:
    kfree(koa.ops);

    return err;
}

static const struct file_operations fops =
{
    .owner = THIS_MODULE,
    .open = msrbatch_open,
    .unlocked_ioctl = msrbatch_ioctl,
    .compat_ioctl = msrbatch_ioctl,
    .release = msrbatch_close
};

void msrbatch_cleanup(int majordev)
{
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
        unregister_chrdev(majordev, "cpu/msr_batch");
    }
}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,39)
static char *msrbatch_nodename(struct device *dev, mode_t *mode)
#else
static char *msrbatch_nodename(struct device *dev, umode_t *mode)
#endif
{
    return kasprintf(GFP_KERNEL, "cpu/msr_batch");
}

int msrbatch_init(int *majordev)
{
    int err = 0;
    struct device *dev;

    err = register_chrdev(*majordev, "cpu/msr_batch", &fops);
    if (err < 0)
    {
        pr_debug("%s: unable to register chrdev\n", __FUNCTION__);
        msrbatch_cleanup(*majordev);
        return -EBUSY;
    }
    if (err > 0)
    {
        *majordev = err;
    }
    cdev_registered = 1;

    cdev_class = class_create(THIS_MODULE, "msr_batch");
    if (IS_ERR(cdev_class))
    {
        err = PTR_ERR(cdev_class);
        msrbatch_cleanup(*majordev);
        return err;
    }
    cdev_class_created = 1;

    cdev_class->devnode = msrbatch_nodename;

    dev = device_create(cdev_class, NULL, MKDEV(*majordev, 0), NULL, "msr_batch");
    if (IS_ERR(dev))
    {
        err = PTR_ERR(dev);
        msrbatch_cleanup(*majordev);
        return err;
    }
    cdev_created = 1;
    return 0;
}
