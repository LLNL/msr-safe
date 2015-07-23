/*
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
	__u32 isread;			/* non-zero for rdmsr, zero for wrmsr */
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

#define X86_IOC_MSR_BATCH	_IOWR('c', 0xA2, struct msr_bundle_desc)

#ifdef __KERNEL__
int msr_safe_bundle(struct msr_bundle_desc *k_bdes);
#endif /* __KERNEL__ */
#endif /*  MSR_HFILE_INC */
