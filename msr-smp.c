/*
#  Copyright (c) 2011, 2012, 2013, 2014, 2015 by Lawrence Livermore National Security, LLC. LLNL-CODE-645430
#  Produced at the Lawrence Livermore National Laboratory.
#  Written by Marty McFadden, Kathleen Shoga and Barry Rountree (mcfadden1|shoga1|rountree@llnl.gov).
#  All rights reserved.
#
#  This file is part of msr-safe.
#
#  msr-safe is free software: you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation, either
#  version 3 of the License, or (at your option) any
#  later version.
#
#  msr-safe is distributed in the hope that it will be useful, but
#  WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along
#  with msr-safe. If not, see <http://www.gnu.org/licenses/>.
#
#  This material is based upon work supported by the U.S. Department
#  of Energy's Lawrence Livermore National Laboratory. Office of
#  Science, under Award number DE-AC52-07NA27344.
*/
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
