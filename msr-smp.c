/*
 * (proposed) extensions to arch/x86/lib/msr_smp.c
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

struct msr_bundle_info {
	int err;
	struct msr_bundle_desc *bd;
};

static void __msr_safe_bundle(void *info)
{
	struct msr_bundle_info *bi = info;
	struct msr_bundle_desc *bd = bi->bd;
	int this_cpu = smp_processor_id();
	struct msr_cpu_ops *ops;
	struct msr_op *op;
	int err = 0;

	for (ops = bd->bundle; ops < bd->bundle + bd->n_msr_bundles; ++ops) {
		if (ops->cpu != this_cpu)
			continue;

		for (op = &ops->ops[0]; op < &ops->ops[ops->n_ops]; ++op) {
			if (op->isread)
				op->errno = rdmsr_safe(op->msr, &op->d.d32[0],
								 &op->d.d32[1]);
			else
				op->errno = wrmsr_safe(op->msr, op->d.d32[0],
								 op->d.d32[1]);
			if (op->errno)
				err = op->errno;
		}
	}
	bi->err = err;
}

int msr_safe_bundle(struct msr_bundle_desc *bd)
{
	struct cpumask cpus_to_run_on;
	int i;
	struct msr_bundle_info bi;
	bi.bd = bd;
	bi.err = -EIO;

	cpumask_clear(&cpus_to_run_on);
	for (i = 0; i < bd->n_msr_bundles; ++i)
		cpumask_set_cpu(bd->bundle[i].cpu, &cpus_to_run_on);

	on_each_cpu_mask(&cpus_to_run_on, __msr_safe_bundle, &bi, 1);
	
	return bi.err;
}
