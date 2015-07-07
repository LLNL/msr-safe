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
#include <asm/msr.h>
#include "msr.h"

