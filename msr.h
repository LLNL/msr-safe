/*
 * (proposed) extensions to arch/x86/include/asm/msr.h
 *
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

#define MSR_MAX_BATCH_OPS	50	/* Maximum ops per logical CPU */

struct msr_op {
	union msrdata {
		__u32 d32[2];		/* For lo/hi access */
		__u64 d64;
	} d;
	__u64 mask;			/* Used by kernel */
	__u32 msr;			/* MSR Address to perform operatio */
	int errno;			/* zero indicates success */
};

struct msr_cpu_ops {
	__u32 cpu;			/* Logical CPU # */
	int n_ops;			/* # of operations for this CPU */
	struct msr_op ops[MSR_MAX_BATCH_OPS];
};

struct msr_bundle_desc {
	int n_msr_bundles;			/* # of jobs in batch */
	struct msr_cpu_ops *bundle;	/* Bundle of msr ops for CPUs */
};

#define X86_IOC_RDMSR_BATCH	_IOWR('c', 0xA2, struct msr_bundle_desc)
#define X86_IOC_WRMSR_BATCH	_IOWR('c', 0xA3, struct msr_bundle_desc)

#ifdef __KERNEL__
int rdmsr_safe_bundle(struct msr_bundle_desc *k_bdes);
int wrmsr_safe_bundle(struct msr_bundle_desc *k_bdes);
#endif /* __KERNEL__ */
#endif /*  MSR_HFILE_INC */
