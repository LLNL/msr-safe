// csr_whitelist.h
// Author: Scott Walker
// Description: whitelist for csr access

#ifndef _ARCH_X86_KERNEL_CSR_WHITELIST_H
#define _ARCH_X86_KERNEL_CSR_WHITELIST_H 1

#include <linux/types.h>
#include <linux/hashtable.h>

#include "csr.h"

struct whitelist_entry {
	// the write mask for the CSR
	__u64 wmask;
	// bits 11:0 CSR (offset)
	// bits 15:12 function
	// bits 20:16 device
	// bits 23:21 reserved, must be 0
	// bits 32:24 bus
	// bits 63:32 inherits wmask from this csr
	__u64 csr;
	// the data of the csr
	__u64 *csrdata;
	// the iomap for this register on socket 0
	void __iomem *maps0;
	// the iomap for this register on socket 1
	void __iomem *maps1;
	struct hlist_node hlist;
};

int csr_whitelist_init(void);
void csr_whitelist_cleanup(void);
struct whitelist_entry *csr_whitelist_entry(struct csr_batch_op *op);
void __iomem *csr_whitelist_map(__u8 socket, __u8 bus, __u8 device, __u8 function, loff_t reg);

#endif /*_ARCH_X86_KERNEL_CSR_WHITELIST_H */
