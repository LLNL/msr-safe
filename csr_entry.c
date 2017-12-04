/*
 *
 * Copyright (c) 2015-2017, Lawrence Livermore National Security, LLC.
 * LLNL-CODE-645430
 *
 * Produced at Lawrence Livermore National Laboratory
 * Written by  Barry Rountree, rountree@llnl.gov
 *             Scott Walker,   walker91@llnl.gov
 *             Marty McFadden, mcfadden8@llnl.gov
 *             Kathleen Shoga, shoga1@llnl.gov
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
 *
 */

#include <linux/buffer_head.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/version.h>

#include "csr_safe.h"
#include "csr_whitelist.h"

static __u64 BAR = 0x0; /* have to hard code for now */
static __u16 ARCH = 0x0;
static __u32 BAR_PCI_DID = 0x0; /* base address register device ID */
static __u32 BAR_PCI_OFFT = 0x0; /* offset from the beginning of the configuration space */
//static __u64 LIM = 0xCC000000;
static void __iomem *b0d5f0 = 0x0;
static void __iomem *b80d5f0 = 0x0;
static int majordev;
static struct class *cdev_class;
static char cdev_created;
static char cdev_registered;
static char cdev_class_created;

struct pci_dev *pdev;

struct csrbatch_session_info
{
    int rawio_allowed;
};

static void *__resource_start(__u8 bus,
                              __u8 device,
                              __u8 func);

static int csrsafe_open(struct inode *inode,
                        struct file *filp);

static int csrsafe_validate(void __iomem *bdf_socket);

static int csrsafe_close(struct inode *inode,
                         struct file *filp);

static int __init csr_init(void);

static void __exit csr_exit(void);

static long csr_ioctl(struct file *filp,
                      unsigned int ioc,
                      unsigned long arg);

static int __csr_batch(struct csr_batch_array *oa,
                       struct csrbatch_session_info *myinfo);

static __u64 set_bar(void);

static __u64 get_cpuid_mf(void);


static const struct file_operations fops =
{
    .owner = THIS_MODULE,
    .open = csrsafe_open,
    .unlocked_ioctl = csr_ioctl,
    .release = csrsafe_close
};

static __u64 get_cpuid_mf(void)
{
    __u64 rax = 1, rbx = 0, rcx = 0, rdx = 0;
    asm volatile("cpuid"
                 : "=a" (rax),
                 "=b" (rbx),
                 "=c" (rcx),
                 "=d" (rdx)
                 : "0" (rax),
                 "2" (rcx));
    return 0x0 | ((rax >> 4) & 0xF) | ((rax >> 12) & 0xF0);
}

static void *__resource_start(__u8 bus, __u8 device, __u8 func)
{
    void *res = (void *)(BAR + (bus * 1024 * 1024 + device * 32 * 1024 + func * 4 * 1024));
    //if (res > (void *) LIM || res < (void *) BAR)
    //{
    //    printk("%s Error: requested resource out of pcie config space\n", __NODENAME);
    //    res = (void *)(BAR + (0x0 * 1024 * 1024 + 0x5 * 32 * 1024 + 0x0 * 4 * 1024));
    //}
    return res;
}

void __iomem *__pcie_ioremap(__u8 bus, __u8 device, __u8 func)
{
    return ioremap((resource_size_t) __resource_start(bus, device, func), 4096);
}

__u8 csr_core_bus(const __u32 socket)
{
    __u8 data = 0;
    __u32 reg = 0x108;
    void __iomem *bdf_socket = (socket ? b80d5f0 : b0d5f0);
    if (!bdf_socket)
    {
        printk("%s error: attempted dereference null bdf (core)\n", __NODENAME);
        return -EIO;
    }
    data = ioread8(bdf_socket + reg);
    return data;
}

__u8 csr_uncore_bus(const __u32 socket)
{
    __u8 data = 0;
    __u32 reg = 0x108;
    void __iomem *bdf_socket = (socket ? b80d5f0 : b0d5f0);
    if (!bdf_socket)
    {
        printk("%s error: attempted dereference null bdf (uncore)\n", __NODENAME);
        return -EIO;
    }
    data = ioread8(bdf_socket + reg + 1);
    return data;
}

static int csrsafe_open(struct inode *inode, struct file *filp)
{
    struct csrbatch_session_info *myinfo;
    myinfo = kmalloc(sizeof(*myinfo), GFP_KERNEL);
    if (!myinfo)
    {
        return -ENOMEM;
    }

    myinfo->rawio_allowed = capable(CAP_SYS_RAWIO);
    filp->private_data = myinfo;
    printk("%s you have opened version %s\n", __NODENAME, __VERSTRING);

    return 0;
}

static int __csr_batch(struct csr_batch_array *oa, struct csrbatch_session_info *myinfo)
{
    struct csr_batch_op *op;
    struct whitelist_entry *entry;
    void __iomem *map = NULL;
    __u32 tempdata[2] = {0x0, 0x0};
    int err = 0;

    for (op = oa->ops; op < oa->ops + oa->numops; ++op)
    {
        entry = csr_whitelist_entry(op);
        if (!entry)
        {
            pr_err("%s No whitelist entry for CSR %x on b%dd%df%d\n", __NODENAME, op->offset, op->bus, op->device, op->function);
            op->err = err = -EACCES;
            continue;
        }
        else
        {
            if (myinfo->rawio_allowed)
            {
                op->wmask = 0xFFFFFFFFFFFFFFFF;
            }
            else
            {
                op->wmask = entry->wmask;
            }
            if (op->socket)
            {
                map = entry->maps1;
            }
            else
            {
                map = entry->maps0;
            }
        }
        if (op->isread)
        {
            op->csrdata = 0x0;
            if (op->size <= 4)
            {
                tempdata[0] = ioread32(map + op->offset);
            }
            else
            {
                ioread32_rep(map + op->offset, tempdata, 2);
            }
            op->csrdata = (__u64) *tempdata;
        }
        else
        {
            if (op->wmask == 0 && !op->isread)
            {
                pr_err("%s CSR %x on b%dd%df%d is read-only\n", __NODENAME, op->offset, op->bus, op->device, op->function);
            }
            tempdata[0] = (u32) (op->csrdata & op->wmask);
            iowrite32((__u32) tempdata[0], map + op->offset);
            if (op->wmask & 0xFFFFFFFF00000000)
            {
                tempdata[1] = (__u32) ((op->csrdata & op->wmask) >> 32);
                iowrite32((__u32) tempdata[1], map + op->offset + 4);
            }
        }
    }
    return err;
}

static long csr_ioctl(struct file *filp, unsigned int ioc, unsigned long arg)
{
    int err = 0;
    struct csr_batch_array __user *uarg;
    struct csr_batch_op __user *uops;
    struct csr_batch_array karg;
    struct csrbatch_session_info *myinfo = filp->private_data;

    if (ioc != CSRSAFE_8086_BATCH)
    {
        pr_err("Invalid ioctl op %u\n", ioc);
        return -EINVAL;
    }

    if (!(filp->f_mode & FMODE_READ))
    {
        pr_err("File not open for reading\n");
        return -EBADF;
    }

    uarg = (struct csr_batch_array *)arg;

    if (copy_from_user(&karg, uarg, sizeof(karg)))
    {
        pr_err("Copy of batch array descriptor failed\n");
        return -EFAULT;
    }

    if (!karg.numops)
    {
        pr_err("Invalid number of ops %d\n", karg.numops);
        return -EINVAL;
    }

    uops = karg.ops;
    karg.ops = kmalloc_array(karg.numops, sizeof(*karg.ops), GFP_KERNEL);
    if (!karg.ops)
    {
        return -ENOMEM;
    }

    if (copy_from_user(karg.ops, uops, karg.numops * sizeof(*karg.ops)))
    {
        pr_err("Copy of batch array failed\n");
        err = -EFAULT;
        kfree(karg.ops);
        return err;
    }

    err = __csr_batch(&karg, myinfo);

    if (copy_to_user(uops, karg.ops, karg.numops * sizeof(*uops)))
    {
        pr_err("copy batch data back to user failed\n");
        if (!err)
        {
            err = -EFAULT;
        }
    }
    kfree(karg.ops);
    return err;
}

static int csrsafe_validate(void __iomem *bdf_socket)
{
    __u32 data = 0;
    __u32 reg = 0x0;

    data = ioread32(bdf_socket + reg);

    if (data != (BAR_PCI_DID << 16 | 0x8086))
    {
        printk("%s ERROR: validation data is %x\n", __NODENAME, data);
        printk("%s Error: invalid installation of module. Aborting...\n", __NODENAME);
        return -1;
    }
    data = ioread32(bdf_socket + BAR_PCI_OFFT);
    if (data != BAR)
    {
        printk("%s ERROR: invalid BAR. Aborting...\n", __NODENAME);
        return -1;
    }
    printk("%s finished validating...\n", __NODENAME);
    return 0;
}

static __u64 set_bar(void)
{
    __u32 tempbar[2] = {0x0, 0x0};
    __u64 mf = get_cpuid_mf();
    int err;

    switch (mf)
    {
        case 0x3E:
            BAR_PCI_DID = 0xe28;
            ARCH = 0x063E;
            BAR_PCI_OFFT = 0x84;
            break;
        case 0x3F:
            BAR_PCI_DID = 0x2F28;
            ARCH = 0x063F;
            BAR_PCI_OFFT = 0x90;
            break;
        case 0x2D:
            BAR_PCI_DID = 0x3C28;
            ARCH = 0x062D;
            BAR_PCI_OFFT = 0x84;
            break;
        default:
            pr_err("%s unsupported architecture. Aborting...\n", __NODENAME);
            printk("%s found %llx\n", __NODENAME, mf);
            return -1;
    }
    printk("%s architecture is %x\n", __NODENAME, ARCH);

    pdev = pci_get_device(0x8086, BAR_PCI_DID, NULL);
    if (pdev)
    {
        err = pci_enable_device(pdev);
        if (err)
        {
            pr_err("%s unable to enable pci_dev (master)\n", __NODENAME);
            return -PCIENER;
        }
        pci_read_config_dword(pdev, BAR_PCI_OFFT, &tempbar[0]);
        pci_read_config_dword(pdev, BAR_PCI_OFFT + 4, &tempbar[1]);
        switch (ARCH)
        {
            case 0x063E:
            case 0x062D:
                BAR = ((__u64) *tempbar) & 0xFC000000;
                break;
            case 0x063F:
                BAR = ((__u64) *tempbar) & 0xFC000000;
                break;
            default:
                pr_err("%s wrong architecture. This is bad.\n", __NODENAME);
                return -PCIENER;
        }
    }
    else
    {
        pr_err("%s unable to open pci_dev\n", __NODENAME);
        return -PCIENER;
    }
    printk("%s BAR set\n", __NODENAME);

    return 0;
}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 39)
static char *csrsafe_nodename(struct device *dev, mode_t *mode)
#else
static char *csrsafe_nodename(struct device *dev, umode_t *mode)
#endif
{
    /* this is a pcie device but the data is actually from the processor */
    return kasprintf(GFP_KERNEL, CSRSAFE_DIR);
}

static int csrsafe_close(struct inode *inode, struct file *filp)
{
    kfree(filp->private_data);
    filp->private_data = 0;
    return 0;
}

static void csrsafe_cleanup(void)
{
    if (cdev_created)
    {
        if (b0d5f0)
        {
            iounmap(b0d5f0);
        }
        if (b80d5f0)
        {
            iounmap(b80d5f0);
        }
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
        unregister_chrdev(majordev, CSRSAFE_DIR);
    }
}

static int __init csr_init(void)
{
    int err = 0;
    struct device *dev;

    majordev = register_chrdev(0, CSRSAFE_DIR, &fops);
    if (majordev < 0)
    {
        pr_err("csrsafe_init: unable to register chrdev\n");
        csrsafe_cleanup();
        return -EBUSY;
    }
    cdev_registered = 1;

    cdev_class = class_create(THIS_MODULE, "csr_safe");
    if (IS_ERR(cdev_class))
    {
        err = PTR_ERR(cdev_class);
        csrsafe_cleanup();
        return err;
    }
    cdev_class_created = 1;

    cdev_class->devnode = csrsafe_nodename;

    dev = device_create(cdev_class, NULL, MKDEV(majordev, 0), NULL, "csr_safe");
    if (IS_ERR(dev))
    {
        err = PTR_ERR(dev);
        csrsafe_cleanup();
        return err;
    }
    cdev_created = 1;

    err = set_bar();
    if (err)
    {
        csrsafe_cleanup();
        return err;
    }

    b0d5f0 = __pcie_ioremap(0x0, 5, 0x0);
    if (csrsafe_validate(b0d5f0))
    {
        csrsafe_cleanup();
        err = PTR_ERR(b0d5f0);
        return err;
    }
    b80d5f0 = __pcie_ioremap(0x80, 5, 0x0);
    if (csrsafe_validate(b80d5f0))
    {
        csrsafe_cleanup();
        err = PTR_ERR(b80d5f0);
        return err;
    }

    err = csr_whitelist_init();
    if (err)
    {
        pr_err("failed to initialize whitelist for csr\n");
        csr_whitelist_cleanup();
        return -1;
    }

    return 0;
}

static void __exit csr_exit(void)
{
    csrsafe_cleanup();
    csr_whitelist_cleanup();
}

module_init(csr_init);
module_exit(csr_exit)

MODULE_AUTHOR("Scott Walker <walker91@llnl.gov>");
MODULE_DESCRIPTION("pcie config space safe access driver");
MODULE_LICENSE("GPL");
