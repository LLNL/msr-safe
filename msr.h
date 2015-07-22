/*
 * This file defines linux kernel data structures and function prototypes
 * for proposed extensions to arch/x86/lib/msr_smp.c that will allow for
 * batching rdmsr/wrmsr requests.
 *
 * Marty McFadden <mcfadden8@llnl.gov>
 */
#ifndef MSR_HFILE_INC
#define MSR_HFILE_INC 1
#include <linux/types.h>
#include <linux/ioctl.h>

struct msr_batch_rdmsr_op {
	__u16 err;		/* Out: set if error occuurred with this op */
	__u16 cpu;		/* In: CPU to execute {rd/wr}msr ins. */
	__u32 msr;		/* In: MSR Address to perform op */
	__u64 msrdata;		/* In/Out: Input/Result to/from operation */
};

struct msr_batch_rdmsr_array {
	int numops;		/* In: # of operations in ops array */
	struct msr_batch_rdmsr_op *ops;	/* In: Array[numops] of operations */
};

#define X86_IOC_MSR_RDMSR_BATCH	_IOWR('c', 0xA2, struct msr_batch_rdmsr_array)

#ifdef __KERNEL__
int rdmsr_safe_batch(struct msr_batch_rdmsr_array *oa);
#endif /* __KERNEL__ */
#endif /*  MSR_HFILE_INC */
