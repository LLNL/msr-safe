/*
 * MSR Whitelist implementation
 */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/hashtable.h>
#include <linux/mutex.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/ctype.h>
#include <linux/device.h>

#define MAX_WLIST_BSIZE ((128 * 1024) + 1) /* "+1" for null character */

struct whitelist_entry {
	u64 wmask;	/* Bits that may be written */
	u64 msr;	/* Address of msr (used as hash key) */
	u64 *msrdata;	/* ptr to original msr contents of writable bits */
	struct hlist_node hlist;
};

static void delete_whitelist(void);
static int create_whitelist(int nentries);
static struct whitelist_entry *find_in_whitelist(u64 msr);
static void add_to_whitelist(struct whitelist_entry *entry);
static int parse_next_whitelist_entry(char *inbuf, char **nextinbuf,
						struct whitelist_entry *entry);
static ssize_t read_whitelist(struct file *file, char __user *buf,
						size_t count, loff_t *ppos);
static int majordev;
static struct class *cdev_class;
static char cdev_created;
static char cdev_registered;
static char cdev_class_created;

static DEFINE_HASHTABLE(whitelist_hash, 6);
static DEFINE_MUTEX(whitelist_mutex);
static struct whitelist_entry *whitelist;
static int whitelist_numentries;

int msr_whitelist_maskexists(loff_t reg)
{
	struct whitelist_entry *entry;

	mutex_lock(&whitelist_mutex);
	entry = find_in_whitelist((u64)reg);
	mutex_unlock(&whitelist_mutex);

	return entry != NULL;
}

u64 msr_whitelist_writemask(loff_t reg)
{
	struct whitelist_entry *entry;

	mutex_lock(&whitelist_mutex);
	entry = find_in_whitelist((u64)reg);
	mutex_unlock(&whitelist_mutex);

	return entry ? entry->wmask : 0;
}

static int open_whitelist(struct inode *inode, struct file *file)
{
	return 0;
}

/*
 * After copying data from user space, we make two passes through it.
 * The first pass is to ensure that the input file is valid. If the file is
 * valid, we will then delete the current white list and then perform the
 * second pass to actually create the new white list.
 */
static ssize_t write_whitelist(struct file *file, const char __user *buf,
						size_t count, loff_t *ppos)
{
	int err = 0;
	const u32 __user *tmp = (const u32 __user *)buf;
	char *s;
	int res;
	int num_entries;
	struct whitelist_entry *entry;
	char *kbuf;

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
			if (find_in_whitelist(entry->msr)) {
				pr_err("write_whitelist: Duplicate: %llx\n",
							 entry->msr);
				err = -EINVAL;
				delete_whitelist();
				goto out_releasemutex;
			}
			add_to_whitelist(entry);
		}
	}

out_releasemutex:
	mutex_unlock(&whitelist_mutex);
out_freebuffer:
	kfree(kbuf);
	return err ? err : count;
}

static ssize_t read_whitelist(struct file *file, char __user *buf,
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
		"MSR: %08llx Write Mask: %016llx\n", e.msr, e.wmask);

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
	if (whitelist == 0)
		return;

	if (whitelist->msrdata != 0)
		kfree(whitelist->msrdata);

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

static struct whitelist_entry *find_in_whitelist(u64 msr)
{
	struct whitelist_entry *entry = 0;

	if (whitelist) {
		hash_for_each_possible(whitelist_hash, entry, hlist, msr)
			if (entry && entry->msr == msr)
				return entry;
	}
	return 0;
}

static void add_to_whitelist(struct whitelist_entry *entry)
{
	hash_add(whitelist_hash, &entry->hlist, entry->msr);
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
		entry->msr = data[0];
		entry->wmask = data[1];
	}

	*nextinbuf = s; /* Return where we left off to caller */
	return *nextinbuf - inbuf;
}

static char *msr_whitelist_nodename(struct device *dev, umode_t *mode)
{
	return kasprintf(GFP_KERNEL, "cpu/msr_whitelist");
}

void msr_whitelist_cleanup(void)
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
		unregister_chrdev(majordev, "cpu/msr_whitelist");
	}
}

int msr_whitelist_init(void)
{
	int err;
	struct device *dev;

	majordev = register_chrdev(0, "cpu/msr_whitelist", &fops);
	if (majordev < 0) {
		pr_err("msr_whitelist_init: unable to register chrdev\n");
		msr_whitelist_cleanup();
		return -EBUSY;
	}
	cdev_registered = 1;

	cdev_class = class_create(THIS_MODULE, "msr_whitelist");
	if (IS_ERR(cdev_class)) {
		err = PTR_ERR(cdev_class);
		msr_whitelist_cleanup();
		return err;
	}
	cdev_class_created = 1;

	cdev_class->devnode = msr_whitelist_nodename;

	dev = device_create(cdev_class, NULL, MKDEV(majordev, 0),
						NULL, "msr_whitelist");
	if (IS_ERR(dev)) {
		err = PTR_ERR(dev);
		msr_whitelist_cleanup();
		return err;
	}
	cdev_created = 1;
	return 0;
}
