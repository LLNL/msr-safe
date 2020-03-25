// Copyright 2011-2020 Lawrence Livermore National Security, LLC and other
// msr-safe Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: LGPL-2.1-only

/*
 * This file enables batching rdmsr/wrmsr requests.
 */

#include <asm/msr.h>
#include <linux/cpumask.h>
#include <linux/module.h>
#include <linux/preempt.h>
#include <linux/smp.h>

#include "msr_safe.h"

static void __msr_safe_batch(void *info)
{
    struct msr_batch_array *arr = info;
    struct msr_batch_op *op;
    int this_cpu = smp_processor_id();
    u32 *dp;
    u64 oldmsr;
    u64 newmsr;

    for (op = arr->ops; op < arr->ops + arr->numops; ++op)
    {
        if (op->cpu != this_cpu)
        {
            continue;
        }

        op->err = 0;
        dp = (u32 *)&oldmsr;
        if (rdmsr_safe(op->msr, &dp[0], &dp[1]))
        {
            op->err = -EIO;
            continue;
        }
        if (op->isrdmsr)
        {
            op->msrdata = oldmsr;
            continue;
        }

        newmsr = op->msrdata & op->wmask;
        newmsr |= (oldmsr & ~op->wmask);
        dp = (u32 *)&newmsr;
        if (wrmsr_safe(op->msr, dp[0], dp[1]))
        {
            op->err = -EIO;
        }
    }
}

int msr_safe_batch(struct msr_batch_array *arr)
{
    struct cpumask cpus_to_run_on;
    struct msr_batch_op *op;

    cpumask_clear(&cpus_to_run_on);
    for (op = arr->ops; op < arr->ops + arr->numops; ++op)
    {
        cpumask_set_cpu(op->cpu, &cpus_to_run_on);
    }

    on_each_cpu_mask(&cpus_to_run_on, __msr_safe_batch, arr, 1);

    for (op = arr->ops; op < arr->ops + arr->numops; ++op)
    {
        if (op->err)
        {
            return op->err;
        }
    }

    return 0;
}
