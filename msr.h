#ifndef MSR_SAFE_MSRH
#define MSR_SAFE_MSRH 1
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

extern int rdmsr_safe_sg_on_cpu(unsigned int cpu, u32 msr_no, u32 *l, u32 *h);
extern int wrmsr_safe_sg_on_cpu(unsigned int cpu, u32 msr_no, u32 l, u32 h);
#endif /* MSR_SAFE_MSRH */
