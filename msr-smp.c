/*
 * Copyright (c) 2011-2017 by Lawrence Livermore National Security, LLC.
 * LLNL-CODE-645430
 *
 * Produced at Lawrence Livermore National Laboratory.
 * Written by  Marty McFadden, mcfadden8@llnl.gov
 *             Kathleen Shoga, shoga1@llnl.gov
 *             Barry Rountree, rountree@llnl.gov
 *
 * All rights reserved.
 *
 * This file is part of msr-safe.
 *
 * msr-safe is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * msr-safe is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with msr-safe. If not, see <http://www.gnu.org/licenses/>.
 *
 * This material is based upon work supported by the U.S. Department of
 * Energy's Lawrence Livermore National Laboratory. Office of Science, under
 * Award number DE-AC52-07NA27344.
 */

/*
 * (proposed) extensions to arch/x86/lib/msr_smp.c
 *
 * This file is the implementation proposed to arch/x86/lib/msr_smp.c
 * that will allow for batching rdmsr/wrmsr requests.
 */

#include <asm/msr.h>
#include <linux/cpumask.h>
#include <linux/module.h>
#include <linux/preempt.h>
#include <linux/smp.h>

#include "msr_safe.h"

static void __msr_safe_batch(void *info)
{
    struct msr_batch_array *oa = info;
    struct msr_batch_op *op;
    int this_cpu = smp_processor_id();
    u32 *dp;
    u64 oldmsr;
    u64 newmsr;

    for (op = oa->ops; op < oa->ops + oa->numops; ++op)
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

int msr_safe_batch(struct msr_batch_array *oa)
{
    struct cpumask cpus_to_run_on;
    struct msr_batch_op *op;

    cpumask_clear(&cpus_to_run_on);
    for (op = oa->ops; op < oa->ops + oa->numops; ++op)
    {
        cpumask_set_cpu(op->cpu, &cpus_to_run_on);
    }

    on_each_cpu_mask(&cpus_to_run_on, __msr_safe_batch, oa, 1);

    for (op = oa->ops; op < oa->ops + oa->numops; ++op)
    {
        if (op->err)
        {
            return op->err;
        }
    }

    return 0;
}
