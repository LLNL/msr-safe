// Copyright 2011-2021 Lawrence Livermore National Security, LLC and other
// msr-safe Project Developers. See the top-level COPYRIGHT file for
// details.
//
// SPDX-License-Identifier: GPL-2.0-only

/*
 * (proposed) extensions to arch/x86/lib/msr_smp.c
 *
 * This file is the implementation proposed to arch/x86/lib/msr_smp.c
 * that will allow for batching rdmsr/wrmsr requests.
 */

#include <asm/msr.h>
#include <asm/msr-index.h>
#include <linux/cpumask.h>
#include <linux/module.h>
#include <linux/preempt.h>
#include <linux/smp.h>

#include "msr_safe.h"
#include "msr-smp.h"

static void __msr_safe_batch(void *info)
{
    struct msr_batch_array *oa = info;
    struct msr_batch_op *op;
    int this_cpu = smp_processor_id();
    u64 rdval, wrval, mperfval, aperfval, pollval;
    u32 *rdptr=(u32 *)&rdval, *wrptr=(u32 *)&wrval, *mperfptr=(u32 *)&mperfval, *aperfptr=(u32 *)&aperfval, *pollptr=(u32 *)&pollval;

    for (op = oa->ops; op < oa->ops + oa->numops; ++op)
    {
        if (op->cpu != this_cpu)
        {
            continue;
        }

        op->err = 0;

        if ( op->op & OP_INITIAL_MPERF ){
            if ( rdmsr_safe( MSR_IA32_MPERF, &mperfptr[0], &mperfptr[1] )){
                op->err = -EIO;
                continue;
            }
            op->mperf_initial = mperfval;
        }
        if ( op->op & OP_INITIAL_APERF ){
            if ( rdmsr_safe( MSR_IA32_APERF, &aperfptr[0], &aperfptr[1] )){
                op->err = -EIO;
                continue;
            }
            op->aperf_initial = aperfval;
        }
        if ( op->op & OP_WRITE || op->op & OP_READ || op->op & OP_POLL ){
            if ( rdmsr_safe( op->msr, &rdptr[0], &rdptr[1] )){
                op->err = -EIO;
                continue;
            }
            if ( op->op & OP_READ || op->op & OP_POLL ){
                op->msrdata = rdval;
            }
        }
        if ( op->op & OP_WRITE ){
            wrval = op->msrdata & op->wmask;
            wrval |= rdval & ~op->wmask;
            if ( wrmsr_safe( op->msr, wrptr[0], wrptr[1] ) ){
                op->err = -EIO;
                continue;
            }
        }
        if ( op->op & OP_POLL ){
            while(1){
                if( 0 == op->poll_max ){
                    break;
                }
                op->poll_max--;
                if ( op->op & OP_POLL_MPERF ){
                    if ( rdmsr_safe( MSR_IA32_MPERF, &mperfptr[0], &mperfptr[1] )){
                        op->err = -EIO;
                        continue;
                    }
                    op->mperf_poll = mperfval;
                }
                if ( op->op & OP_POLL_APERF ){
                    if ( rdmsr_safe( MSR_IA32_APERF, &aperfptr[0], &aperfptr[1] )){
                        op->err = -EIO;
                        continue;
                    }
                    op->aperf_poll = mperfval;
                }
                if ( rdmsr_safe( op->msr, &pollptr[0], &pollptr[1] )){
                    op->err = -EIO;
                    continue;
                }
                if ( pollval != rdval ){
                    op->msrdata2 = pollval;
                    break;
                }
            }
        }
        if ( op->op & OP_FINAL_APERF ){
            if ( rdmsr_safe( MSR_IA32_APERF, &aperfptr[0], &aperfptr[1] )){
                op->err = -EIO;
                continue;
            }
            op->mperf_final = aperfval;
        }
        if ( op->op & OP_FINAL_MPERF ){
            if ( rdmsr_safe( MSR_IA32_MPERF, &mperfptr[0], &mperfptr[1] )){
                op->err = -EIO;
                continue;
            }
            op->mperf_final = mperfval;
        }
    }
}

#ifdef CONFIG_CPUMASK_OFFSTACK

int msr_safe_batch(struct msr_batch_array *oa)
{
    cpumask_var_t cpus_to_run_on;
    struct msr_batch_op *op;

    if (!zalloc_cpumask_var(&cpus_to_run_on, GFP_KERNEL))
    {
        return -ENOMEM;
    }

    for (op = oa->ops; op < oa->ops + oa->numops; ++op)
    {
        cpumask_set_cpu(op->cpu, cpus_to_run_on);
    }

    on_each_cpu_mask(cpus_to_run_on, __msr_safe_batch, oa, 1);

    free_cpumask_var(cpus_to_run_on);

    for (op = oa->ops; op < oa->ops + oa->numops; ++op)
    {
        if (op->err)
        {
            return op->err;
        }
    }

    return 0;
}

#else

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

#endif //CONFIG_CPUMASK_OFFSTACK
