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
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/fs.h>       // Needed for file i/o
#include <linux/device.h>
#include <linux/cpu.h>
#include <linux/uaccess.h>
#include <linux/ctype.h>

/*
 * The minor device scheme works as follows:
 *      0 through (NR_CPUS-1) are for the CPUs
 *      NR_CPUS is the administrative whitelist interface
 */
#define MSR_NUM_MINORS          NR_CPUS+1
#define MSR_WLIST_ADMIN_MINOR   NR_CPUS
#define MSR_IS_WLIST_ADMIN(m)   ((m) == MSR_WLIST_ADMIN_MINOR)

static struct class *msr_class;
static int majordev;    /* Our dynamically allocated major device number */

/* Initialization flags */
static char msr_chardev_created[MSR_NUM_MINORS];
static char msr_chardev_registered = 0;
static char msr_class_created = 0;
static char msr_notifier_registered = 0;

/* Data buffering for parsing user input */
#define MAX_WLIST_BSIZE (64 * 1024)
static char msrbuf[MAX_WLIST_BSIZE+1];   /* "+1" for the null character */

/* White list management section */
typedef struct {
    u64 wmask;  /* Bits that may be written */
    u64 rmask;  /* Bits that may be read */
    u64 msr;    /* Address of msr (used as hash key) */
    struct hlist_node hlist;
} msr_twlist;

static void msrwl_delete(void)
{
    /* TODO */
}

static int msrwl_create(int nentries)
{
    int err = 0;

    msrwl_delete(); /* Replace current whitelist */

    /* TODO */
    return err;
}

static int msrwl_find(msr_twlist *entry)
{
    int found = 0;

    /* TODO */
    return found;
}

static void msrwl_add(msr_twlist *entry)
{
    /* TODO */
}

static int msrwl_parse_entry(char *inbuf, char **nextinbuf, msr_twlist *entry)
{
    char *s = skip_spaces(inbuf);
    int i;
    u64 data[3];

    while (*s == '#') {   /* Skip remaining portion of line */
        for (s = s + 1; *s && *s != '\n'; s++)
            ;
        s = skip_spaces(s);
    }

    if (*s == 0)
        return 0;            /* This means we are done with the input buffer */

    for (i = 0; i < 3; i++) {/* Okay, we should have the first of 3 #s now */
        char *s2; 
        int err;
        char tmp;

        s2 = s = skip_spaces(s);
        while (!isspace(*s) && *s)
            s++;

        if (*s == 0) {
            printk(KERN_ALERT "msrwl_parse_entry: Premature EOF");
            return -EINVAL;
        }

        tmp = *s;
        *s = 0;   /* Null-terminate this portion of string */
        if ((err = kstrtoull(s2, 0, &data[i]))) {
            printk(KERN_ALERT "msrwl_parse_entry kstrtoull(%s) failed(%d)\n", 
                    s2, err);
            return err;
        }
        *s++ = tmp;
    }

    entry->msr =  data[0];
    entry->wmask = data[1];
    entry->rmask = data[2];

    *nextinbuf = s;     /* Return where we left off to caller */
    return *nextinbuf - inbuf;
}

static ssize_t msrwl_update(struct file *file, const char __user *buf, 
                                                size_t count, loff_t *ppos)
{
    const u32 __user *tmp = (const u32 __user *)buf;
    char *s;
    int res;
    int num_entries;
    msr_twlist entry;

    printk(KERN_DEBUG "msrwl_update: %s:%i, %zu buffer provided\n", 
                                                    __FILE__, __LINE__, count);

    /* TODO: Remove this restriction and handle large files like a man... */
    if (count > MAX_WLIST_BSIZE) {
        printk(KERN_ALERT "msrwl_update: Data buffer of %zu bytes too large\n",
                                                                         count);
        return -EINVAL;
    }

    if (copy_from_user(msrbuf, tmp, count)) {
        printk(KERN_ALERT "msrwl_update: copy_from_user(%zu bytes) failed\n", 
                                                                        count);
        return -EFAULT;
    }

    msrbuf[count] = 0;  // NULL-terminate to make it into a big string

    /*
     * We make two passes through the file.  The first pass is to ensure that
     * the input file is valid.  If the file is valid, we will then delete the
     * current white list and then perform the second pass to actually 
     * create the new white list.
     *
     * Pass 1:
     */
    for (num_entries = 0, s = msrbuf, res = 1; res > 0; ) {
        if ((res = msrwl_parse_entry(s, &s, &entry)) < 0)
            return res;         /* Parsing failed */

        if (res) {
            num_entries++;
            printk(KERN_DEBUG "msrw_update: Entry(%llx %llx %llx) parsed\n", 
                                        entry.msr, entry.wmask, entry.rmask);
        }
    }

    printk(KERN_DEBUG "msrwl_update: %d entries parsed\n", num_entries);

    if ((res = msrwl_create(num_entries)) < 0) {
        printk(KERN_ALERT "msrwl_update: Failed to allocate %d entries\n", 
                                                                num_entries);
        return res;
    }

    /*
     * Pass 2:
     */
    for (num_entries = 0, s = msrbuf, res = 1; res > 0; ) {
        if ((res = msrwl_parse_entry(s, &s, &entry)) < 0) {
            return res;         /* This should not happen! */
        }
        if (res) {
            num_entries++;
            printk(KERN_DEBUG "msrw_update: Entry(%llx %llx %llx) parsed\n", 
                                        entry.msr, entry.wmask, entry.rmask);
        }
    }
    return count;
}

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

static ssize_t 
msr_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    u32 __user *tmp = (u32 __user *) buf;
    u32 data[2];
    loff_t reg = *ppos;
    int cpu = iminor(file->f_path.dentry->d_inode);
    int err = -EACCES; /* Initialize to Permission Denied */

    if (MSR_IS_WLIST_ADMIN(cpu)) {
        printk(KERN_DEBUG "Admin: %s:%i, %zu buffer provided\n", 
                                        __FILE__, __LINE__, count);
        return count-1;
    }

    /* "Count" doesn't have any meaning here, as we
     * never want to read more than one msr at at time.
     */
    if (count != 8)
        return -EINVAL;	/* Invalid chunk size */

    /* TODO: Put whitelist check here */
    err = rdmsr_safe_on_cpu(cpu, reg, &data[0], &data[1]);
    if (!err)
    {
        if (copy_to_user(tmp, &data, 8))
            err = -EFAULT;
        else
            err = 0; /* Success */
    }
    return err ? err : 8;
}

static ssize_t 
msr_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    if (MSR_IS_WLIST_ADMIN(iminor(file->f_path.dentry->d_inode))) {
        return msrwl_update(file, buf, count, ppos);
    }
    else {
        const u32 __user *tmp = (const u32 __user *)buf;
        u32 data[2];
        loff_t reg = *ppos;
        int cpu = iminor(file->f_path.dentry->d_inode);
        int err = -EACCES;

        /* "Count" doesn't have any meaning here, as we
         * never want to write more than one msr at at time.
         */
        if (count != 8)
            return -EINVAL;	/* Invalid chunk size */

        /* TODO: Put whitelist check here
         * TODO: Need to determine how to best return partial write success 
         *       which would happen if the user had write access to some, 
         *       but not all bit fields.
         */
        if (copy_from_user(&data[0], tmp, 8)) {
            err = -EFAULT;
        }
        else {
            /*TODO: data &= MASK;*/
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

    dev = device_create(msr_class, NULL, MKDEV(majordev, cpu), 
                        NULL, "msr_safe%d", cpu);
    return IS_ERR(dev) ? PTR_ERR(dev) : 0;
}

static void msr_device_destroy(int cpu)
{
    device_destroy(msr_class, MKDEV(majordev, cpu));
}

static int __cpuinit 
msr_class_cpu_callback(struct notifier_block *nfb, 
                        unsigned long action, void *hcpu)
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

static void msr_cleanup(void)
{
    int cpu = 0;

    msrwl_delete();

    if (msr_notifier_registered) {
        msr_notifier_registered = 0;
        unregister_hotcpu_notifier(&msr_class_cpu_notifier);
    }

    for_each_online_cpu(cpu) {
        if (msr_chardev_created[cpu]) {
            msr_chardev_created[cpu] = 0;
            msr_device_destroy(cpu);
        }
    }

    if (msr_chardev_created[MSR_WLIST_ADMIN_MINOR]) {
        msr_chardev_created[MSR_WLIST_ADMIN_MINOR] = 0;
        msr_device_destroy(MSR_WLIST_ADMIN_MINOR);
    }

    if (msr_class_created) {
        msr_class_created = 0;
        class_destroy(msr_class);
    }

    if (msr_chardev_registered) {
        msr_chardev_registered = 0;
        __unregister_chrdev(majordev, 0, MSR_NUM_MINORS, "cpu/msr_safe");
    }
}

static int __init msr_init(void)
{
    int i, err;

    majordev = __register_chrdev(0,0,MSR_NUM_MINORS, "cpu/msr_safe", &msr_fops);
    if (majordev < 0) {
        printk(KERN_ERR "msr_safe: unable to register device number\n");
        msr_cleanup();
        return -EBUSY;
    }
    msr_chardev_registered = 1;

    msr_class = class_create(THIS_MODULE, "msr_safe");
    if (IS_ERR(msr_class)) {
        err = PTR_ERR(msr_class);
        msr_cleanup();
        return err;
    }
    msr_class_created = 1;

    for (i = 0; i < MSR_NUM_MINORS; i++)
        msr_chardev_created[i] = 0;

    msr_class->devnode = msr_devnode;
    if ((err = msr_device_create(MSR_WLIST_ADMIN_MINOR))) {
        msr_cleanup();
        return err;
    }
    msr_chardev_created[MSR_WLIST_ADMIN_MINOR] = 1;
       
    i = 0;
    for_each_online_cpu(i) {
        if ((err = msr_device_create(i)) != 0) {
            msr_cleanup();
            return err;
        }
        msr_chardev_created[i] = 1;
    }

    register_hotcpu_notifier(&msr_class_cpu_notifier);
    msr_notifier_registered = 1;
    return 0;
}

static void __exit msr_exit(void)
{
    msr_cleanup();
}

module_init(msr_init);
module_exit(msr_exit)

MODULE_AUTHOR("Marty McFadden <mcfadden8@llnl.gov>");
MODULE_DESCRIPTION("x86 sanitized MSR driver");
MODULE_LICENSE("GPL");
MODULE_SUPPORTED_DEVICE("msr_safe");

