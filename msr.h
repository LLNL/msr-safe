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

struct msr_batch_op {
	__u16 cpu;		/* In: CPU to execute {rd/wr}msr ins. */
	__u16 isrdmsr;		/* In: 0=wrmsr, non-zero=rdmsr */
	__s32 err;		/* Out: set if error occurred with this op */
	__u32 msr;		/* In: MSR Address to perform op */
	__u64 msrdata;		/* In/Out: Input/Result to/from operation */
	__u64 wmask;		/* Out: Write mask applied to wrmsr */
};

struct msr_batch_array {
	__u32 numops;			/* In: # of operations in ops array */
	struct msr_batch_op *ops;	/* In: Array[numops] of operations */
};

#define X86_IOC_MSR_BATCH	_IOWR('c', 0xA2, struct msr_batch_array)

#ifdef __KERNEL__
int msr_safe_batch(struct msr_batch_array *oa);
#endif /* __KERNEL__ */
#endif /*  MSR_HFILE_INC */
