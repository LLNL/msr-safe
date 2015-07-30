#ifndef _UAPI_ASM_X86_MSR_H
#define _UAPI_ASM_X86_MSR_H

#ifndef __ASSEMBLY__

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

#define X86_IOC_RDMSR_REGS	_IOWR('c', 0xA0, __u32[8])
#define X86_IOC_WRMSR_REGS	_IOWR('c', 0xA1, __u32[8])
#define X86_IOC_MSR_BATCH	_IOWR('c', 0xA2, struct msr_batch_array)

#endif /* __ASSEMBLY__ */
#endif /* _UAPI_ASM_X86_MSR_H */
