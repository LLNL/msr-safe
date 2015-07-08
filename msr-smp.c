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

static struct cpumask cpus_to_run_on;
int rdmsr_safe_bundle(struct msr_bundle_desc *k_bdes)
{
	cpumask_clear(&cpus_to_run_on);

	/* Todo: Loop through and set up CPUs we need to run on */
	//cpumask_set_cpu(cpu, &cpus_to_run_on);
	//on_each_cpu_mask(&cpus_to_run_on, __ourcallback, k_bdes, 1);

	
	return 0;
}
