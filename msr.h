/*
<<<<<<< HEAD
=======
#  Copyright (c) 2011, 2012, 2013, 2014, 2015 by Lawrence Livermore National Security, LLC. LLNL-CODE-645430
#  Produced at the Lawrence Livermore National Laboratory.
#  Written by Marty McFadden, Kathleen Shoga and Barry Rountree (mcfadden1|shoga1|rountree@llnl.gov).
#  All rights reserved.
#
#  This file is part of msr-safe.
#
#  msr-safe is free software: you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation, either
#  version 3 of the License, or (at your option) any
#  later version.
#
#  msr-safe is distributed in the hope that it will be useful, but
#  WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along
#  with msr-safe. If not, see <http://www.gnu.org/licenses/>.
#
#  This material is based upon work supported by the U.S. Department
#  of Energy's Lawrence Livermore National Laboratory. Office of
#  Science, under Award number DE-AC52-07NA27344.
*/
/*
 * (proposed) extensions to arch/x86/include/asm/msr.h
 *
>>>>>>> d7ccb3efe28f6d08882197cd71549354598d025f
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
