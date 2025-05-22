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

typedef enum : uint16_t {
    OP_WRITE            = 0x0001,
    OP_READ             = 0x0002,
    OP_POLL             = 0x0004,
    OP_MPERF            = 0x0008,

    OP_APERF            = 0x0010,
    OP_TSC              = 0x0020,
    OP_THERM            = 0x0040,
    OP_PTHERM           = 0x0080,

    // The DELTA* flags are hints for userspace postprocessing
    // and are ignored by the kernel.
    DELTA_MPERF         = 0x0100,
    DELTA_APERF         = 0x0200,
    DELTA_TSC           = 0x0400,
    DELTA_THERM         = 0x0800,

    DELTA_PTHERM        = 0x1000,
    DELTA_MSRDATA       = 0x2000,
    MAX_OP_VAL          = 0x4000,
} op_flag_t;

#define OP_ALL_MODS         ( OP_MPERF | OP_APERF | OP_TSC | OP_THERM | OP_PTHERM )

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
                            //   DELTA_*    Userspace postprocessing.  Ignored by the kernel.
                            //
                            //
    __s32 err;              // Out: set if error occurred with this operation
    __u32 poll_max;         // In/Out:  Max/remaining poll attempts
    __u32 msr;              // In: MSR Address to perform operation
    __u64 wmask;            // Out: Write mask applied to value used in OP_WRITE
    __u64 msrdata;          // In/Out: Input/Result to/from operation

    // The following are only populated if the relevant op flags are set
    __u64 msrdata2;         // Out: last polled reading
    __u64 tsc;              // Out: time stamp counter at op completion
    __u64 mperf;            // Out: MPERF value at op completion
    __u64 aperf;            // Out: APERF value at op completion
    __u64 therm;            // Out: THERM_STATUS
    __u64 ptherm;           // Out: PACKAGE_THERM_STATUS
    __u64 tag;              // Userspace accounting, not used by the kernel
};

struct msr_batch_array
{
    __u32 numops;               // In: # of operations in operations array
    __u32 version;              // In: MSR_SAFE_VERSION_u32 (see msr_version.h)
    struct msr_batch_op *ops;   // In: Array[numops] of operations
};

#ifdef MSR_SAFE_USERSPACE
// Requires --std=c23 and <stdint.h>
typedef enum : uint64_t {
    op_field_arridx_CPU,
    op_field_arridx_OP,
    op_field_arridx_ERR,
    op_field_arridx_POLL_MAX,
    op_field_arridx_WMASK,
    op_field_arridx_MSR,
    op_field_arridx_MSRDATA,
    op_field_arridx_MSRDATA2,
    op_field_arridx_MPERF,
    op_field_arridx_APERF,
    op_field_arridx_TSC,
    op_field_arridx_THERM,
    op_field_arridx_PTHERM,
    op_field_arridx_TAG,
    // The remainder do not appear in the msr_batch_op struct
    op_field_arridx_DELTA_MPERF,
    op_field_arridx_DELTA_APERF,
    op_field_arridx_DELTA_TSC,
    op_field_arridx_DELTA_THERM,
    op_field_arridx_DELTA_PTHERM,
    op_field_arridx_DELTA_MSRDATA,
    op_field_arridx_MAX_IDX,

} op_field_arridx_t;

typedef enum : uint64_t {
    op_field_bitidx_CPU             = 1 << op_field_arridx_CPU,
    op_field_bitidx_OP              = 1 << op_field_arridx_OP,
    op_field_bitidx_ERR             = 1 << op_field_arridx_ERR,
    op_field_bitidx_POLL_MAX        = 1 << op_field_arridx_POLL_MAX,
    op_field_bitidx_WMASK           = 1 << op_field_arridx_WMASK,
    op_field_bitidx_MSR             = 1 << op_field_arridx_MSR,
    op_field_bitidx_MSRDATA         = 1 << op_field_arridx_MSRDATA,
    op_field_bitidx_MSRDATA2        = 1 << op_field_arridx_MSRDATA2,
    op_field_bitidx_MPERF           = 1 << op_field_arridx_MPERF,
    op_field_bitidx_APERF           = 1 << op_field_arridx_APERF,
    op_field_bitidx_TSC             = 1 << op_field_arridx_TSC,
    op_field_bitidx_THERM           = 1 << op_field_arridx_THERM,
    op_field_bitidx_PTHERM          = 1 << op_field_arridx_PTHERM,
    op_field_bitidx_TAG             = 1 << op_field_arridx_TAG,
    // The remainder do not appear in the msr_batch_op struct
    op_field_bitidx_DELTA_MPERF     = 1 << op_field_arridx_DELTA_MPERF,
    op_field_bitidx_DELTA_APERF     = 1 << op_field_arridx_DELTA_APERF,
    op_field_bitidx_DELTA_TSC       = 1 << op_field_arridx_DELTA_TSC,
    op_field_bitidx_DELTA_THERM     = 1 << op_field_arridx_DELTA_THERM,
    op_field_bitidx_DELTA_PTHERM    = 1 << op_field_arridx_DELTA_PTHERM,
    op_field_bitidx_DELTA_MSRDATA   = 1 << op_field_arridx_DELTA_MSRDATA,
    op_field_bitidx_MAX_IDX         = 1 << op_field_arridx_MAX_IDX
} op_field_bitidx_t;

static const char * const opfield2str[ op_field_arridx_MAX_IDX ] = {
    [op_field_arridx_CPU            ] = "cpu",
    [op_field_arridx_OP             ] = "op",
    [op_field_arridx_ERR            ] = "err",
    [op_field_arridx_POLL_MAX       ] = "poll_max",
    [op_field_arridx_WMASK          ] = "wmask",
    [op_field_arridx_MSR            ] = "msr",
    [op_field_arridx_MSRDATA        ] = "msrdata",
    [op_field_arridx_MSRDATA2       ] = "msrdata2",
    [op_field_arridx_MPERF          ] = "mperf",
    [op_field_arridx_APERF          ] = "aperf",
    [op_field_arridx_TSC            ] = "tsc",
    [op_field_arridx_THERM          ] = "therm",
    [op_field_arridx_PTHERM         ] = "ptherm",
    [op_field_arridx_TAG            ] = "tag",
    // The remainder do not appear in the msr_batch_op struct
    [op_field_arridx_DELTA_MPERF    ] = "dmperf",
    [op_field_arridx_DELTA_APERF    ] = "daperf",
    [op_field_arridx_DELTA_TSC      ] = "dtsc",
    [op_field_arridx_DELTA_THERM    ] = "dtherm",
    [op_field_arridx_DELTA_PTHERM   ] = "dptherm",
    [op_field_arridx_DELTA_MSRDATA  ] = "dmsrdata",
};

static const char * const opflags2str[ MAX_OP_VAL ] = {
    [OP_WRITE]      = "OP_WRITE",
    [OP_READ]       = "OP_READ",
    [OP_POLL]       = "OP_POLL",
    [OP_MPERF]      = "OP_MPERF",
    [OP_APERF]      = "OP_APERF",
    [OP_TSC]        = "OP_TSC",
    [OP_THERM]      = "OP_THERM",
    [OP_PTHERM]     = "OP_PTHERM",
    [DELTA_MPERF]   = "DELTA_MPERF",
    [DELTA_APERF]   = "DELTA_APERF",
    [DELTA_TSC]     = "DELTA_TSC",
    [DELTA_THERM]   = "DELTA_THERM",
    [DELTA_PTHERM]  = "DELTA_PTHERM",
    [DELTA_MSRDATA] = "DELTA_MSRDATA",
};


#endif // MSR_SAFE_USERSPACE

#define X86_IOC_MSR_BATCH   _IOWR('c', 0xA2, struct msr_batch_array)

#endif
