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
#define OP_MPERF            0x0008
#define OP_APERF            0x0010
#define OP_TSC              0x0020
#define OP_THERM            0x0040
#define OP_PTHERM           0x0080

struct msr_batch_op
{
    __u16 cpu;              // In: CPU to execute {rd/wr}msr instruction
                            //
    __u16 op;               // In: These may be OR'ed together.
                            //   OP_WRITE   Write the value of msrdata (anded with the allowlist writemask) the msr.
                            //
                            //   OP_READ    Read the value of the msr into msrdata.
                            //
                            //   OP_POLL    Read the value of the msr into msrdata, then continue reading until the
                            //              value changes and record the new value into msrdata2.  If poll_max tries
                            //              have been attempted without the value changing, msrdata2 is not modified.
                            //
                            //              It's possible to combine these three ops, but doing so doesn't necessarily
                            //              make sense.  msr-safe will first read (regardless of the op), then write,
                            //              then poll.  OP_READ | OP_POLL will be indistinguishable from OP_POLL.
                            //              OP_READ | OP_WRITE will be indistinguishable from OP_WRITE, except that
                            //              msrdata will end up with the (writemasked) written value.
                            //
                            //   OP_TSC     Record the tsc "reference cycles" after the write/read/poll op(s) complete.
                            //
                            //   OP_MPERF   Record accumulated reference cycles after the w/r/p op(s) complete.
                            //
                            //   OP_APERF   Record accumulated actual cycles after the w/r/p op(s) complete.
                            //
                            //   OP_THERM   Record the core temperature (degrees C away from TCC)
                            //
                            //   OP_PTHERM  Record the package temperature (degrees C away from TCC)
                            //
    __s32 err;              // Out: set if error occurred with this operation
    __u32 poll_max;         // In/Out:  Max/remaining poll attempts
    __u32 msr;              // In: MSR Address to perform operation
    __u64 msrdata;          // In/Out: Input/Result to/from operation
    __u64 msrdata2;         // Out: last polled reading
    __u64 wmask;            // Out: Write mask applied to value used in OP_WRITE
    __u64 tsc;              // Out: time stamp counter at op completion
    __u64 mperf;            // Out: MPERF value at op completion
    __u64 aperf;            // Out: APERF value at op completion
    __u64 therm;            // Out: THERM_STATUS
    __u64 ptherm;           // Out: PACKAGE_THERM_STATUS
};

struct msr_batch_array
{
    __u32 numops;               // In: # of operations in operations array
    __u32 version;              // In: MSR_SAFE_VERSION_u32 (see msr_version.h)
    struct msr_batch_op *ops;   // In: Array[numops] of operations
};

#define X86_IOC_MSR_BATCH   _IOWR('c', 0xA2, struct msr_batch_array)

#endif
