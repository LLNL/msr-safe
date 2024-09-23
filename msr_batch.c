// Copyright 2011-2021 Lawrence Livermore National Security, LLC and other
// msr-safe Project Developers. See the top-level COPYRIGHT file for
// details.
//
// SPDX-License-Identifier: GPL-2.0-only

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
#include "msr-smp.h"
#include "msr_allowlist.h"

static struct class *cdev_class;
static char cdev_created;
static char cdev_registered;
static char cdev_class_created;

static int msrbatch_apply_allowlist(struct msr_batch_array *oa)
{
    struct msr_batch_op *op;
    int err = 0;

    for (op = oa->ops; op < oa->ops + oa->numops; ++op)
    {
        op->err = 0;

        if (op->cpu >= nr_cpu_ids || !cpu_online(op->cpu))
        {
            pr_debug("No such CPU %d\n", op->cpu);
            op->err = err = -ENXIO; // No such CPU
            continue;
        }

        if (!msr_allowlist_maskexists(op->msr))
        {
            pr_debug("No allowlist entry for MSR %x\n", op->msr);
            op->err = err = -EACCES;
        }
        else
        {
            op->wmask = msr_allowlist_writemask(op->msr);
            /* Check for read-only case */
            if (op->wmask == 0 && !op->isrdmsr)
            {
                pr_debug("MSR %x is read-only\n", op->msr);
                op->err = err = -EROFS;
            }
        }
    }
    return err;
}

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
        return -E2BIG;
    }

    if (copy_from_user(koa.ops, uops, koa.numops * sizeof(*koa.ops)))
    {
        pr_debug("Copy of batch array failed\n");
        err = -EFAULT;
        goto bundle_alloc;
    }

    err = msrbatch_apply_allowlist(&koa);
    if (err)
    {
        pr_debug("Failed to apply allowlist %d\n", err);
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
    .unlocked_ioctl = msrbatch_ioctl,
    .compat_ioctl = msrbatch_ioctl
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

#define msrbatch_nodename_selector _Generic(\
    (((struct class *)0)->devnode),\
    char* (*) (      struct device *,  mode_t *) : msrbatch_nodename1,\
    char* (*) (      struct device *, umode_t *) : msrbatch_nodename2,\
    char* (*) (const struct device *, umode_t *) : msrbatch_nodename3 \
    )

static char *msrbatch_nodename1(struct device *dev, mode_t *mode)
{
    return kasprintf(GFP_KERNEL, "cpu/msr_batch");
}

static char *msrbatch_nodename2(struct device *dev, umode_t *mode)
{
    return kasprintf(GFP_KERNEL, "cpu/msr_batch");
}

static char *msrbatch_nodename3(const struct device *dev, umode_t *mode)
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

    cdev_class = class_create(
#if LINUX_VERSION_CODE < KERNEL_VERSION(6,4,0)
                    THIS_MODULE,
#endif
                    "msr_batch");
    if (IS_ERR(cdev_class))
    {
        err = PTR_ERR(cdev_class);
        msrbatch_cleanup(*majordev);
        return err;
    }
    cdev_class_created = 1;

    cdev_class->devnode = msrbatch_nodename_selector;

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
