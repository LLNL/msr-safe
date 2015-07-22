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

static void __rdmsr_safe_batch(void *info)
{
	struct msr_batch_rdmsr_array *oa = info;
	struct msr_batch_rdmsr_op *op;
	int this_cpu = smp_processor_id();
	u32 *d;

	for (op = oa->ops; op < oa->ops + oa->numops; ++op) {
		if (op->cpu != this_cpu)
			continue;

		d = (u32 *)&op->msrdata;
		op->err = rdmsr_safe(op->msr, &d[0], &d[1]);
	}
}

int rdmsr_safe_batch(struct msr_batch_rdmsr_array *oa)
{
	struct cpumask cpus_to_run_on;
	struct msr_batch_rdmsr_op *op;

	cpumask_clear(&cpus_to_run_on);
	for (op = oa->ops; op < oa->ops + oa->numops; ++op)
		cpumask_set_cpu(op->cpu, &cpus_to_run_on);

	on_each_cpu_mask(&cpus_to_run_on, __rdmsr_safe_batch, oa, 1);
	
	for (op = oa->ops; op < oa->ops + oa->numops; ++op)
		if (op->err)
			return op->err;

	return 0;
}
