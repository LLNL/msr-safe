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
 * This driver uses /dev/cpu/%d/msr_safe where %d is the minor number, and on
 * an SMP box will direct the access to CPU %d.
 */
#include <linux/module.h>   // Needed by all kernel modules
#include <linux/fs.h>       // Needed for file i/o
#include <linux/device.h>
#include <linux/cpu.h>
#include <linux/uaccess.h>

static struct class *msr_class;
static int majordev;                // Our dynamically allocated major device number
/*
 * The minor device scheme works as follows:
 *      0 through (NR_CPUS-1) are for the CPUs
 *      NR_CPUS is the administrative whitelist interface
 */
#define MSR_NUM_MINORS  NR_CPUS+1
#define MSR_WLIST_ADMIN_MINOR   NR_CPUS
#define MSR_IS_WLIST_ADMIN(m)   ((m) == MSR_WLIST_ADMIN_MINOR)

static loff_t msr_seek(struct file *file, loff_t offset, int orig)
{
    loff_t ret;
    struct inode *inode = file->f_mapping->host;

    mutex_lock(&inode->i_mutex);
    switch (orig)
    {
    case 0:
        file->f_pos = offset;
        ret = file->f_pos;
        break;
    case 1:
        file->f_pos += offset;
        ret = file->f_pos;
        break;
    default:
        ret = -EINVAL;
    }
    mutex_unlock(&inode->i_mutex);
    return ret;
}

static ssize_t msr_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    u32 __user *tmp = (u32 __user *) buf;
    u32 data[2];
    loff_t reg = *ppos;
    int cpu = iminor(file->f_path.dentry->d_inode);
    int err = -EACCES; //Initialize to Permission Denied

    if (MSR_IS_WLIST_ADMIN(cpu)) {
        printk(KERN_DEBUG "Admin: %s:%i, %d buffer provided\n", __FILE__, __LINE__, (int)count);
        return count-1;
    }

    // "Count" doesn't have any meaning here, as we
    // never want to read more than one msr at at time.
    if (count != 8)
        return -EINVAL;	/* Invalid chunk size */

    // TODO: Put whitelist check here
    err = rdmsr_safe_on_cpu(cpu, reg, &data[0], &data[1]);
    if (!err)
    {
        if (copy_to_user(tmp, &data, 8))
            err = -EFAULT;
        else
            err = 0; //Success
    }
    return err ? err : 8;
}

#define MAX_WLIST_BSIZE (64 * 1024)
typedef struct {
    u64 wmask;  // Bits that may be written
    u64 rmask;  // Bits that may be read
    u32 msr;    // Address of msr
} msr_twlist;

static ssize_t msr_whitelist_update(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    const u32 __user *tmp = (const u32 __user *)buf;
    u32 data[2];
    loff_t reg = *ppos;
    if (count > MAX_WLIST_BSIZE) {
        printk(KERN_ALERT "msr_whitelist_update: Data buffer of %d bytes is too large\n", (int)count);
        return -EINVAL;
    }
    // TODO: Left off here.
    // TODO: Easiest thing would be to clear previous whitelist
    //       and then write a new one.
    // TODO; Parse input
    //

    printk(KERN_DEBUG "msr_whitelist_update: %s:%i, %d buffer provided\n", __FILE__, __LINE__, (int)count);
    return count;
}

static ssize_t msr_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    if (MSR_IS_WLIST_ADMIN(iminor(file->f_path.dentry->d_inode))) {
        return msr_whitelist_update(file, buf, count, ppos);
    }
    else {
        const u32 __user *tmp = (const u32 __user *)buf;
        u32 data[2];
        loff_t reg = *ppos;
        int cpu = iminor(file->f_path.dentry->d_inode);
        int err = -EACCES;

        // "Count" doesn't have any meaning here, as we
        // never want to write more than one msr at at time.
        if (count != 8)
            return -EINVAL;	/* Invalid chunk size */

        // TODO: Put whitelist check here
        // TODO: Need to determine how to best return partial write success which
        //       would happen if the user had write access to some, but not all bit fields.
    
        if (copy_from_user(&data[0], tmp, 8)) {
            err = -EFAULT;
        }
        else {
            //TODO: data &= MASK;
            err = wrmsr_safe_on_cpu(cpu, reg, data[0], data[1]);
        }
        return err ? err : 8;
    }
}

static int msr_open(struct inode *inode, struct file *file)
{
    unsigned int cpu;
    struct cpuinfo_x86 *c;

    cpu = iminor(file->f_path.dentry->d_inode);

    if (MSR_IS_WLIST_ADMIN(cpu))
        return 0;       // Let 'em in...

    if (cpu >= nr_cpu_ids || !cpu_online(cpu))
        return -ENXIO;  // No such CPU

    c = &cpu_data(cpu);
    if (!cpu_has(c, X86_FEATURE_MSR))
        return -EIO;	/* MSR not supported */

    return 0;
}

/*
 * File operations we support
 */
static const struct file_operations msr_fops = {
    .owner = THIS_MODULE,
    .read = msr_read,
    .write = msr_write,
    .open = msr_open,
    .llseek = msr_seek
};

static int __cpuinit msr_device_create(int cpu)
{
    struct device *dev;

    dev = device_create(msr_class, NULL, MKDEV(majordev, cpu), NULL, "msr_safe%d", cpu);
    return IS_ERR(dev) ? PTR_ERR(dev) : 0;
}

static void msr_device_destroy(int cpu)
{
    device_destroy(msr_class, MKDEV(majordev, cpu));
}

static int __cpuinit msr_class_cpu_callback(struct notifier_block *nfb, unsigned long action, void *hcpu)
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

static struct notifier_block __refdata msr_class_cpu_notifier = {
    .notifier_call = msr_class_cpu_callback
};

static char *msr_devnode(struct device *dev, umode_t *mode)
{
    if (MSR_IS_WLIST_ADMIN(MINOR(dev->devt)))
        return kasprintf(GFP_KERNEL, "cpu/msr_whitelist");
    else
        return kasprintf(GFP_KERNEL, "cpu/%u/msr_safe", MINOR(dev->devt));
}

static int __init msr_init(void)
{
    int i = 0, err;

    majordev = __register_chrdev(0, 0, MSR_NUM_MINORS, "cpu/msr_safe", &msr_fops);
    if (majordev < 0) {
        printk(KERN_ERR "msr_safe: unable to register device number\n");
        return -EBUSY;
    }

    msr_class = class_create(THIS_MODULE, "msr_safe");
    if (IS_ERR(msr_class)) {
        err = PTR_ERR(msr_class);
        __unregister_chrdev(majordev, 0, MSR_NUM_MINORS, "cpu/msr_safe");
        return err;
    }

    msr_class->devnode = msr_devnode;

    if ((err = msr_device_create(MSR_WLIST_ADMIN_MINOR))) {
            class_destroy(msr_class);
            __unregister_chrdev(majordev, 0, MSR_NUM_MINORS, "cpu/msr_safe");
            return err;
    }
       
    for_each_online_cpu(i) {
        err = msr_device_create(i);
        if (err != 0) {
            // TODO: Need better cleanup here in the event that we die
            // before all devices have been successfully created.
            i = 0;
            for_each_online_cpu(i)
                msr_device_destroy(i);

            msr_device_destroy(MSR_WLIST_ADMIN_MINOR);
            class_destroy(msr_class);
            __unregister_chrdev(majordev, 0, MSR_NUM_MINORS, "cpu/msr_safe");
            return err;
        }
    }
    register_hotcpu_notifier(&msr_class_cpu_notifier);

    return 0;
}

static void __exit msr_exit(void)
{
    int cpu = 0;

    unregister_hotcpu_notifier(&msr_class_cpu_notifier);

    for_each_online_cpu(cpu)
        msr_device_destroy(cpu);

    msr_device_destroy(MSR_WLIST_ADMIN_MINOR);

    class_destroy(msr_class);

    __unregister_chrdev(majordev, 0, MSR_NUM_MINORS, "cpu/msr_safe");
}

module_init(msr_init);
module_exit(msr_exit)

MODULE_AUTHOR("Kathleen Shoga <shoga1@llnl.gov>");
MODULE_DESCRIPTION("x86 sanitized MSR driver");
MODULE_LICENSE("GPL");
MODULE_SUPPORTED_DEVICE("msr_safe");

