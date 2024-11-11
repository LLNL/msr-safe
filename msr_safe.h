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

#define OP_WRITE            0x0001
#define OP_READ             0x0002
#define OP_POLL             0x0004
#define OP_TSC_INITIAL      0x0010
#define OP_TSC_FINAL        0x0020
#define OP_TSC_POLL         0x0040

struct msr_batch_op
{
    __u16 cpu;              // In: CPU to execute {rd/wr}msr instruction
    __u16 op;               // In: OR at least one of the following:
                            //   OP_WRITE, OP_READ, OP_POLL, OP_TSC_INITIAL,
                            //   OP_TSC_FINAL, OP_TSC_POLL
    __s32 err;              // Out: set if error occurred with this operation
    __u32 poll_max;         // In/Out:  Max/remaining poll attempts
    __u32 msr;              // In: MSR Address to perform operation
    __u64 msrdata;          // In/Out: Input/Result to/from operation
    __u64 wmask;            // Out: Write mask applied to wrmsr
    __u64 tsc_initial;      // Out: time stamp counter at op start
    __u64 tsc_final;        // Out: time stamp counter at op completion
    __u64 tsc_poll;         // Out: time stamp counter prior to final poll attempt
    __u64 msrdata2;         // Out: last polled reading
};

struct msr_batch_array
{
    __u32 numops;             // In: # of operations in operations array
    __u32 version;      // In: MSR_SAFE_VERSION_u32 (see msr_version.h)
    struct msr_batch_op *ops; // In: Array[numops] of operations
};

#define X86_IOC_MSR_BATCH   _IOWR('c', 0xA2, struct msr_batch_array)

#endif
