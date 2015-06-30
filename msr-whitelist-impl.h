#ifndef MSR_WHITELIST_IMPL_INC
#define MSR_WHITELIST__IMPL_INC 1
/*
 * implementation specific definitions
 */
#include <linux/kernel.h>

#define MAX_WLIST_BSIZE ((128 * 1024) + 1) /* "+1" for null character */

/*
 * If the msr is writeable and the flag is 0, the msr is erased when the 
 * whitelist is removed.  If the flag is 1 then the value for the msr is 
 * recorded somewhere and restored when the whitelist is removed
 */
#define CLEAR_MSR_ON_WHITELIST_REMOVE(x)	((x) == 1)
#define RESTORE_MSR_ON_WHITELIST_REMOVE(x)	((x) == 2)
struct whitelist_entry {
	u64 wmask;	/* Bits that may be written */
	u64 rmask;	/* Bits that may be read */
	u64 msr;	/* Address of msr (used as hash key) */
	int resflag;	/* 0=do-nothing, 1=erase when removed, 2=restore */
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
#endif /* MSR_WHITELIST_IMPL_INC */
