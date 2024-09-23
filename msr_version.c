// Copyright 2011-2021 Lawrence Livermore National Security, LLC and other
// msr-safe Project Developers. See the top-level COPYRIGHT file for
// details.
//
// SPDX-License-Identifier: GPL-2.0-only

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/device.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/uaccess.h>

#include "msr_version.h"

static struct class *cdev_class;
static char cdev_created;
static char cdev_registered;
static char cdev_class_created;

static int open_version(struct inode *inode, struct file *file)
{
    return 0;
}

static ssize_t read_version(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    size_t len = strlen( THIS_MODULE->version ) + 1 < count ?
        strlen( THIS_MODULE->version ) + 1 :
        count;
    if (*ppos > 0)
    {
        return 0;
    }
    if (len > count)
    {
        return -EFAULT;
    }
    if (copy_to_user(buf, THIS_MODULE->version, len))
    {
        return -EFAULT;
    }
    *ppos = 1;
    return len;
}

static const struct file_operations fops =
{
    .owner = THIS_MODULE,
    .read = read_version,
    .open = open_version
};

#define msr_version_nodename_selector _Generic(\
    (((struct class *)0)->devnode),\
    char * (*) (      struct device *,  mode_t *) : msr_version_nodename1,\
    char * (*) (      struct device *, umode_t *) : msr_version_nodename2,\
    char * (*) (const struct device *, umode_t *) : msr_version_nodename3 \
    )

static char *msr_version_nodename1(struct device *dev, mode_t *mode)
{
    if (mode)
    {
        *mode = 0400;   // read-only
    }
    return kasprintf(GFP_KERNEL, "cpu/msr_safe_version");
}

static char *msr_version_nodename2(struct device *dev, umode_t *mode)
{
    if (mode)
    {
        *mode = 0400;   // read-only
    }
    return kasprintf(GFP_KERNEL, "cpu/msr_safe_version");
}

static char *msr_version_nodename3(const struct device *dev, umode_t *mode)
{
    if (mode)
    {
        *mode = 0400;   // read-only
    }
    return kasprintf(GFP_KERNEL, "cpu/msr_safe_version");
}

void msr_version_cleanup(int majordev)
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
        unregister_chrdev(majordev, "cpu/msr_safe_version");
    }
}

int msr_version_init(int *majordev)
{
    int err = 0;
    struct device *dev;

    err = register_chrdev(*majordev, "cpu/msr_safe_version", &fops);
    if (err < 0)
    {
        pr_debug("%s: unable to register chrdev\n", __FUNCTION__);
        msr_version_cleanup(*majordev);
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
                    "msr_safe_version");
    if (IS_ERR(cdev_class))
    {
        err = PTR_ERR(cdev_class);
        msr_version_cleanup(*majordev);
        return err;
    }
    cdev_class_created = 1;

    cdev_class->devnode = msr_version_nodename_selector;

    dev = device_create(cdev_class, NULL, MKDEV(*majordev, 0), NULL, "msr_safe_version");
    if (IS_ERR(dev))
    {
        err = PTR_ERR(dev);
        msr_version_cleanup(*majordev);
        return err;
    }
    cdev_created = 1;
    return 0;
}
