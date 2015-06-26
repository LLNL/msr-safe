/* ----------------------------------------------------------------------- *
 * Copyright 2000-2008 H. Peter Anvin - All Rights Reserved
 * Copyright 2009 Intel Corporation; author: H. Peter Anvin
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, Inc., 675 Mass Ave, Cambridge MA 02139,
 * USA; either version 2 of the License, or (at your option) any later
 * version; incorporated herein by reference.
 * ----------------------------------------------------------------------- */

/*
 * This file contains an early implementation of a scatter/gather interface
 * to rdmsr and wrmsr.  The intention is for this file to be merged in with
 * lib/msr-smp.c someday.
 */
#include <linux/module.h>
#include <linux/preempt.h>
#include <linux/smp.h>
#include <asm/msr.h>
#include "msr.h"	/* Remove when this goes into msr-smp.c */

static void __rdmsr_safe_sg_on_cpu(void *info)
{
	struct msr_info *rv = info;

	rv->err = rdmsr_safe(rv->msr_no, &rv->reg.l, &rv->reg.h);
}

static void __wrmsr_safe_sg_on_cpu(void *info)
{
	struct msr_info *rv = info;

	rv->err = wrmsr_safe(rv->msr_no, rv->reg.l, rv->reg.h);
}

int rdmsr_safe_sg_on_cpu(unsigned int cpu, u32 msr_no, u32 *l, u32 *h)
{
	int err;
	struct msr_info rv;

	memset(&rv, 0, sizeof(rv));

	rv.msr_no = msr_no;
	err = smp_call_function_single(cpu, __rdmsr_safe_sg_on_cpu, &rv, 1);
	*l = rv.reg.l;
	*h = rv.reg.h;

	return err ? err : rv.err;
}
/* EXPORT_SYMBOL(rdmsr_safe_sg_on_cpu); */

int wrmsr_safe_sg_on_cpu(unsigned int cpu, u32 msr_no, u32 l, u32 h)
{
	int err;
	struct msr_info rv;

	memset(&rv, 0, sizeof(rv));

	rv.msr_no = msr_no;
	rv.reg.l = l;
	rv.reg.h = h;
	err = smp_call_function_single(cpu, __wrmsr_safe_sg_on_cpu, &rv, 1);

	return err ? err : rv.err;
}
/* EXPORT_SYMBOL(wrmsr_safe_sg_on_cpu); */

