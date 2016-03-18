// csr_whitelist.c
// Author: Scott Walker
// Description: a whitelist for csr access

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/pci.h>
#include <linux/version.h>
#include <linux/hashtable.h>
#include <linux/mutex.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/ctype.h>

#include "csr_whitelist.h"
#include "csr.h"

#define MAX_WLIST_BSIZE ((128 * 1023) + 1)

// PROTOTYPES
static void delete_whitelist(void);
static int create_whitelist(int nentries);
static struct whitelist_entry *find_in_whitelist(__u8 bus, __u8 device, __u8 function, __u64 csr);
static struct whitelist_entry *find_in_whitelist_packed(__u64 csr);
static void add_to_whitelist(struct whitelist_entry *entyr);
static int parse_next_whitelist_entry(char *inbuf, char **nextinbuf,
                                      struct whitelist_entry * entry);
static ssize_t read_whitelist(struct file *filp, char __user *buf,
                              size_t count, loff_t *ppos);
// END PROTOTYPES

static int majordev;
static struct class *cdev_class;
static char cdev_created;
static char cdev_registered;
static char cdev_class_created;
static void **maps;
static __u32 maps_array_size;

static DEFINE_HASHTABLE(whitelist_hash, 6);
static DEFINE_MUTEX(whitelist_mutex);

static struct whitelist_entry *whitelist;
static int whitelist_numentries;

struct whitelist_entry *csr_whitelist_entry(struct csr_batch_op *op)
{
	struct whitelist_entry *entry;

	mutex_lock(&whitelist_mutex);
	entry = find_in_whitelist(op->bus, op->device, op->function, op->offset);
	mutex_unlock(&whitelist_mutex);

	return entry ? entry : NULL;
}

static int open_whitelist(struct inode *inode, struct file *filp)
{
    return 0;
}

/*
 * After copying data from user space, we make two passes through it.
 * The first pass is to ensure that the input file is valid. If the file is
 * valid, we will then delete the current white list and then perform the
 * second pass to actually create the new white list.
 */
static ssize_t write_whitelist(struct file *filp, const char __user *buf,
						size_t count, loff_t *ppos)
{
	int err = 0;
	const u32 __user *tmp = (const u32 __user *)buf;
	char *s;
	int res;
	int num_entries;
	struct whitelist_entry *entry;
	struct whitelist_entry *inheritance;
	__u64 prev_csr = 0x0;
	char *kbuf;
	__u8 s0_core_bus = csr_core_bus(0);
	__u8 s0_uncore_bus = csr_uncore_bus(0);
	__u8 s1_core_bus = csr_core_bus(1);
	__u8 s1_uncore_bus = csr_uncore_bus(1);
	void __iomem *lasts0 = 0x0;
	void __iomem *lasts1 = 0x0;
	void **temp = NULL; 
	int itr = 0;

	if (count <= 2) {
		mutex_lock(&whitelist_mutex);
		delete_whitelist();
		hash_init(whitelist_hash);
		mutex_unlock(&whitelist_mutex);
		return count;
	}

	if (count+1 > MAX_WLIST_BSIZE) {
		pr_err("write_whitelist: buffer of %zu bytes too large\n",
		    count);
		return -EINVAL;
	}

	kbuf = kzalloc(count+1, GFP_KERNEL);
	if (!kbuf)
		return -ENOMEM;

	if (copy_from_user(kbuf, tmp, count)) {
		err = -EFAULT;
		goto out_freebuffer;
	}

	/* Pass 1: */
	for (num_entries = 0, s = kbuf, res = 1; res > 0; ) {
		res = parse_next_whitelist_entry(s, &s, 0);
		if (res < 0) {
			err = res;
			goto out_freebuffer;
		}

		if (res)
			num_entries++;
	}

	temp = kmalloc_array(2 * num_entries, sizeof(void __iomem *), GFP_KERNEL);
	if (!temp) {
		goto out_freebuffer;
	}

	/* Pass 2: */
	mutex_lock(&whitelist_mutex);
	res = create_whitelist(num_entries);
	if (res < 0) {
		err = res;
		goto out_releasemutex;
	}

	for (entry = whitelist, s = kbuf, res = 1; res > 0; entry++) {
		res = parse_next_whitelist_entry(s, &s, entry);
		if (res < 0) {
			pr_alert("write_whitelist: Table corrupted\n");
			delete_whitelist();
			err = res; /* This should not happen! */
			goto out_releasemutex;
		}
		if (res) {
			if (find_in_whitelist_packed(entry->csr)) {
				pr_err("write_whitelist: Duplicate: %llx\n",
							 entry->csr);
				err = -EINVAL;
				delete_whitelist();
				goto out_releasemutex;
			}
			if (entry->csr & 0xFFFFFFFF00000000) {
				inheritance = find_in_whitelist((entry->csr >> 56) & 0xFF,
					(entry->csr >> 48) & 0xFF, (entry->csr >> 44) & 0x0F,
					(entry->csr >> 32) & 0xFFF);
				if (inheritance) {
					entry->wmask = inheritance->wmask;
				} else {
					pr_err("%s unable to inherit from nonexistent CSR %llx\n",
						__NODENAME, entry->csr);
					err = -EINVAL;
					delete_whitelist();
					goto out_releasemutex;
				}
			}
			if ((entry->csr & 0x00000000FFFFF000) != prev_csr) {
				// remap
				if (entry->csr & (0x1 << 24)) {
					entry->maps0 = __pcie_ioremap(s0_uncore_bus, 
								(entry->csr >> 16) & 0xFF,
						     		(entry->csr >> 12) & 0x0F);
					lasts0 = temp[itr] = entry->maps0;
					entry->maps1 = __pcie_ioremap(s1_uncore_bus, 
								(entry->csr >> 16) & 0xFF,
						     		(entry->csr >> 12) & 0x0F);
					lasts1 = temp[itr + 1] = entry->maps1;
				} else {
					entry->maps0 = __pcie_ioremap(s0_core_bus, 
								(entry->csr >> 16) & 0xFF,
						     		(entry->csr >> 12) & 0x0F);
					lasts0 = temp[itr] = entry->maps0;
					entry->maps1 = __pcie_ioremap(s1_core_bus, 
								(entry->csr >> 16) & 0xFF,
						     		(entry->csr >> 12) & 0x0F);
					lasts1 = temp[itr + 1] = entry->maps1;
				}
				itr += 2;
			} else {
				entry->maps0 = lasts0;
				entry->maps1 = lasts1;
			}
			if ((long) entry->maps0 == -EIO || (long) entry->maps1 == -EIO) {
				err = -EIO;
				goto out_releasemutex;
			}
			prev_csr = (entry->csr & 0x00000000FFFFF000);
			add_to_whitelist(entry);
		}
	}

	maps_array_size = itr;
	maps = kmalloc_array(maps_array_size, sizeof(void __iomem *), GFP_KERNEL);
	if (!maps) {
		return -ENOMEM;
	}

	for (itr = 0; itr < maps_array_size; itr++) {
		maps[itr] = temp[itr];	
	}

	printk("%s whitelist loaded\n", __NODENAME);

out_releasemutex:
	mutex_unlock(&whitelist_mutex);
out_freebuffer:
	kfree(kbuf);
	if (temp) {
		kfree(temp);
	}
	return err ? err : count;
}

static struct whitelist_entry *find_in_whitelist_packed(__u64 csr) 
{
        __u8 bus = (csr >> 24) & 0xFF;
        __u8 device = (csr >> 16) & 0xFF;
        __u8 function = (csr >> 12) & 0xFF;
        __u64 offset = csr & 0xFFF;

        return find_in_whitelist(bus, device, function, offset);
}

static ssize_t read_whitelist(struct file *filp, char __user *buf,
						size_t count, loff_t *ppos)
{
	loff_t idx = *ppos;
	u32 __user *tmp = (u32 __user *) buf;
	char kbuf[160];
	int len;
	struct whitelist_entry e;

	mutex_lock(&whitelist_mutex);
	*ppos = 0;

	if (idx >= whitelist_numentries || idx < 0) {
		mutex_unlock(&whitelist_mutex);
		return 0;
	}

	e = whitelist[idx];
	mutex_unlock(&whitelist_mutex);

	len = sprintf(kbuf,
		"CSR: %016llx Write Mask: %016llx Map: %p | %p\n", e.csr, e.wmask, e.maps0, e.maps1);

	if (len > count)
		return -EFAULT;

	if (copy_to_user(tmp, kbuf, len))
		return -EFAULT;

	*ppos = idx+1;
	return len;
}

static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = read_whitelist,
	.write = write_whitelist,
	.open = open_whitelist
};

static void delete_whitelist(void)
{
	int itr;
	if (whitelist == 0)
		return;

	for (itr = 0; itr < maps_array_size; itr++) {
		if (maps[itr]) {
			iounmap(maps[itr]);
		}
	}

	if (maps) {
		kfree(maps);
	}

	if (whitelist->csrdata != 0)
		kfree(whitelist->csrdata);

	kfree(whitelist);
	whitelist = 0;
	whitelist_numentries = 0;
}

static int create_whitelist(int nentries)
{
	hash_init(whitelist_hash);
	delete_whitelist();
	whitelist_numentries = nentries;
	whitelist = kcalloc(nentries, sizeof(*whitelist), GFP_KERNEL);

	if (!whitelist)
		return -ENOMEM;
	return 0;
}

static struct whitelist_entry *find_in_whitelist(__u8 bus, __u8 device, __u8 function, u64 csr)
{
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,39)
	struct hlist_node *node;
#endif
	struct whitelist_entry *entry = 0;
        __u64 bdfr = 0x0; // pack bus, device, function, and offset into a quadword
        bdfr = csr | ((__u64) function) << 12 | ((__u64) device) << 16 | ((__u64) bus) << 24;

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,39)
	if (whitelist) {
		hash_for_each_possible(whitelist_hash, entry, node, hlist, bdfr & 0xFFFFFFFF)
			if (entry && (entry->csr & 0xFFFFFFFF) == bdfr) {
				return entry;
			}
	}
#else
	if (whitelist) {
		hash_for_each_possible(whitelist_hash, entry, hlist, bdfr & 0xFFFFFFFF)
			if (entry && (entry->csr & 0xFFFFFFFF) == bdfr) {
				return entry;
			}
	}
#endif
	return 0;
}


static void add_to_whitelist(struct whitelist_entry *entry)
{
	hash_add(whitelist_hash, &entry->hlist, entry->csr & 0xFFFFFFFF);
}

static int parse_next_whitelist_entry(char *inbuf, char **nextinbuf,
						struct whitelist_entry *entry)
{
	char *s = skip_spaces(inbuf);
	int i;
	u64 data[2];

	while (*s == '#') { /* Skip remaining portion of line */
		for (s = s + 1; *s && *s != '\n'; s++)
			;
		s = skip_spaces(s);
	}

	if (*s == 0)
		return 0; /* This means we are done with the input buffer */

	for (i = 0; i < 2; i++) {/* we should have the first of 3 #s now */
		char *s2;
		int err;
		char tmp;

		s2 = s = skip_spaces(s);
		while (!isspace(*s) && *s)
			s++;

		if (*s == 0) {
			pr_err("parse_next_whitelist_entry: Premature EOF");
			return -EINVAL;
		}

		tmp = *s;
		*s = 0; /* Null-terminate this portion of string */
		err = kstrtoull(s2, 0, &data[i]);
		if (err)
			return err;
		*s++ = tmp;
	}

	if (entry) {
		entry->csr = data[0];
		entry->wmask = data[1];
	}

	*nextinbuf = s; /* Return where we left off to caller */
	return *nextinbuf - inbuf;
}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,39)
static char *csr_whitelist_nodename(struct device *dev, mode_t *mode)
#else
static char *csr_whitelist_nodename(struct device *dev, umode_t *mode)
#endif
{
	return kasprintf(GFP_KERNEL, "cpu/csr_whitelist");
}

void csr_whitelist_cleanup(void)
{
	delete_whitelist();

	if (cdev_created) {
		cdev_created = 0;
		device_destroy(cdev_class, MKDEV(majordev, 0));
	}

	if (cdev_class_created) {
		cdev_class_created = 0;
		class_destroy(cdev_class);
	}

	if (cdev_registered) {
		cdev_registered = 0;
		unregister_chrdev(majordev, "cpu/csr_whitelist");
	}
}

int csr_whitelist_init(void)
{
	int err;
	struct device *dev;

	majordev = register_chrdev(0, "cpu/csr_whitelist", &fops);
	if (majordev < 0) {
		pr_err("csr_whitelist_init: unable to register chrdev\n");
		csr_whitelist_cleanup();
		return -EBUSY;
	}
	cdev_registered = 1;

	cdev_class = class_create(THIS_MODULE, "csr_whitelist");
	if (IS_ERR(cdev_class)) {
		err = PTR_ERR(cdev_class);
		csr_whitelist_cleanup();
		return err;
	}
	cdev_class_created = 1;

	cdev_class->devnode = csr_whitelist_nodename;

	dev = device_create(cdev_class, NULL, MKDEV(majordev, 0),
						NULL, "csr_whitelist");
	if (IS_ERR(dev)) {
		err = PTR_ERR(dev);
		csr_whitelist_cleanup();
		return err;
	}
	cdev_created = 1;
	return 0;
}
