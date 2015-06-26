#include <linux/module.h>
#include <linux/preempt.h>
#include <linux/smp.h>
#include <asm/msr.h>

/* These "safe" variants are slower and should be used when the target MSR
   may not actually exist. */
static void __rdmsr_safe_sg_on_cpu(void *info)
{
	struct msr_info *rv = info;
	int i;

	for (i = 0; i < 10000; i++)
		rv->err = rdmsr_safe(rv->msr_no, &rv->reg.l, &rv->reg.h);
}

static void __wrmsr_safe_sg_on_cpu(void *info)
{
	struct msr_info *rv = info;
	int i;

	for (i = 0; i < 10000; i++)
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
EXPORT_SYMBOL(rdmsr_safe_sg_on_cpu);

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
EXPORT_SYMBOL(wrmsr_safe_sg_on_cpu);

