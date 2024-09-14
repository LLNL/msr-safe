// Copyright 2011-2021 Lawrence Livermore National Security, LLC and other
// msr-safe Project Developers. See the top-level COPYRIGHT file for
// details.
//
// SPDX-License-Identifier: GPL-2.0-only

/*
 * This file defines the ioctl interface for submitting a batch of MSR
 * requests.  This file should be distributed to a place where kernel
 * module interface header files are kept for a distribution running this
 * module so that user-space applications and libraries that wish to use
 * the batch interface may #include this file to get the interface.
 */

#ifndef MSR_SAFE_HEADER_INCLUDE
#define MSR_SAFE_HEADER_INCLUDE

#include <linux/ioctl.h>
#include <linux/types.h>

#define OP_WRITE		0x01
#define OP_READ			0x02
#define OP_POLL			0x04
#define OP_INITIAL_MPERF	0x10
#define OP_FINAL_MPERF  	0x20
#define OP_POLL_MPERF		0x40

struct msr_batch_op
{
    __u16 cpu;     	// In: CPU to execute {rd/wr}msr instruction
    __u16 op; 	   	// In: OR at least one of the following:
    			//   OP_WRITE, OP_READ, OP_POLL, OP_INITIAL_MPERF,
    			//   OP_FINAL_MPERF, OP_POLL_MPERF
    __s32 err;     	// Out: set if error occurred with this operation
    __u32 poll_max;	// In/Out:  Max/remaining poll attempts
    __u32 msr;     	// In: MSR Address to perform operation
    __u64 msrdata; 	// In/Out: Input/Result to/from operation
    __u64 wmask;   	// Out: Write mask applied to wrmsr
    __u64 mperf_initial;// Out: reference clock reading at the start of the op
    __u64 mperf_poll;	// Out: reference clock reading at start of final poll
    __u64 mperf_final;	// Out: reference clock reading at the end of r/w/p
    __u64 msrdata2;	// Out: last polled reading
};

struct msr_batch_array
{
    __u32 numops;             // In: # of operations in operations array
    __u32 version;	      // In: MSR_SAFE_VERSION_u32 (see msr_version.h)
    struct msr_batch_op *ops; // In: Array[numops] of operations
};

#define X86_IOC_MSR_BATCH   _IOWR('c', 0xA2, struct msr_batch_array)

#endif
