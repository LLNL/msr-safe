/*
 * extensions to arch/x86/lib/msr_smp.c
 *
 * This file is the implementation proposed to arch/x86/lib/msr_smp.c
 * that will allow for batching rdmsr/wrmsr requests.
 *
 * Marty McFadden <mcfadden8@llnl.gov>
 */
#include <linux/module.h>
#include <linux/preempt.h>
#include <linux/smp.h>
#include <linux/cpumask.h>
#include <asm/msr.h>
#include "msr.h"

static void __msr_safe_batch(void *info)
{
	struct msr_batch_array *oa = info;
	struct msr_batch_op *op;
	int this_cpu = smp_processor_id();
	u32 *dp;
	u64 oldmsr;
	u64 newmsr;

	for (op = oa->ops; op < oa->ops + oa->numops; ++op) {
		if (op->cpu != this_cpu)
			continue;

		op->err = 0;
		dp = (u32 *)&oldmsr;
		if (rdmsr_safe(op->msr, &dp[0], &dp[1])) {
			op->err = -EIO;
			continue;
		}
		if (op->isrdmsr) {
			op->msrdata = oldmsr;
			continue;
		}

		newmsr = op->msrdata & op->wmask;
		newmsr |= (oldmsr & ~op->wmask);
		dp = (u32 *)&newmsr;
		if (wrmsr_safe(op->msr, dp[0], dp[1]))
			op->err = -EIO;
	}
}

int msr_safe_batch(struct msr_batch_array *oa)
{
	struct cpumask cpus_to_run_on;
	struct msr_batch_op *op;

	cpumask_clear(&cpus_to_run_on);
	for (op = oa->ops; op < oa->ops + oa->numops; ++op)
		cpumask_set_cpu(op->cpu, &cpus_to_run_on);

	on_each_cpu_mask(&cpus_to_run_on, __msr_safe_batch, oa, 1);

	for (op = oa->ops; op < oa->ops + oa->numops; ++op)
		if (op->err)
			return op->err;

	return 0;
}
