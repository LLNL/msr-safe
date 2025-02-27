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
#define MSR_TIME_STAMP_COUNTER 0x10
#define THERM_STATUS 0x19c

static void __msr_safe_batch(void *info)
{
    struct msr_batch_array *oa = info;
    struct msr_batch_op *op;
    int this_cpu = smp_processor_id();
    u64 rdval, wrval, tscval, pollval, thermval;
    u32 *rdptr=(u32 *)&rdval, *wrptr=(u32 *)&wrval,
        *tscptr=(u32 *)&tscval,
        *pollptr=(u32 *)&pollval,
        *thermptr=(u32 *)&thermval;

    for (op = oa->ops; op < oa->ops + oa->numops; ++op)
    {
        if (op->cpu != this_cpu)
        {
            continue;
        }

        op->err = 0;
        if ( op->op & OP_TSC_INITIAL ){
            if ( rdmsr_safe( MSR_TIME_STAMP_COUNTER, &tscptr[0], &tscptr[1] )){
                op->err = -EIO;
                continue;
            }
            op->tsc_initial = tscval;
        }
        if ( op->op & OP_THERM_INITIAL ){
            if ( rdmsr_safe( THERM_STATUS, &thermptr[0], &thermptr[1] )){
                op->err = -EIO;
                continue;
            }
            op->therm_initial = thermval;
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
                if ( op->op & OP_TSC_POLL ){
                    if ( rdmsr_safe( MSR_TIME_STAMP_COUNTER, &tscptr[0], &tscptr[1] )){
                        op->err = -EIO;
                        continue;
                    }
                    op->tsc_poll = tscval;
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
        if ( op->op & OP_TSC_FINAL ){
            if ( rdmsr_safe( MSR_TIME_STAMP_COUNTER, &tscptr[0], &tscptr[1] )){
                op->err = -EIO;
                continue;
            }
            op->tsc_final = tscval;
        }
        if ( op->op & OP_THERM_FINAL ){
            if ( rdmsr_safe( THERM_STATUS, &thermptr[0], &thermptr[1] )){
                op->err = -EIO;
                continue;
            }
            op->therm_final = thermval;
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
