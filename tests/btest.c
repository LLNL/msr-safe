/* Program to test out new ioctl interface to /dev/cpu/0/msr_safe */
#include <linux/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <linux/ioctl.h>
#include <unistd.h>
#include "../msr.h"

#define MW(m,v) {{ .d64 = v }, .mask=0, .msr=m, .isread=0, .errno=0}
#define MR(m) {{ .d64 = 0 }, .mask=0, .msr=m, .isread=1, .errno=0}
#define MRCOPY(__CPU__) \
  { .cpu = __CPU__, .n_ops = 33, \
    { MR(0x10), MR(0xE7), MR(0xE8), MR(0x199), MR(0x19A), MR(0x1A0),\
      MR(0x19C), MR(0x19B), MR(0x1A2), MR(0x309), MR(0x30A), MR(0x30B),\
      MR(0x38D), MR(0xC1), MR(0xC2), MR(0xC3), MR(0xC4), MR(0x345), \
      MR(0x38E), MR(0x38F), MR(0x390), MR(0x3F1), MR(0x606), MR(0x610), \
      MR(0x611), MR(0x614), MR(0x618), MR(0x619), MR(0x61B), MR(0x61C), \
      MR(0x638), MR(0x639), MR(0xE7) }\
  }

int batchfd;
int fd[100];
struct msr_cpu_ops bundle[] =
{
  { .cpu = 0, .n_ops = 33,
    {
      MR(0x10),	/* IA32_TIME_STAMP_COUNTER, thread */
#if 0
      MR(0x3B),	/* IA32_TSC_ADJUST, thread */
#endif
      MR(0xE7),	/* IA32_MPERF, varies (unique to thread) */
      MR(0xE8),	/* IA32_APERF, varies (unique to thread) */
      MR(0x199),	/* IA32_PERF_CTL, varies (unique to thread) */
      MR(0x19A),	/* IA32_CLOCK_MODULATION, varies (unique to thread) */
      MR(0x1A0),	/* IA32_MISC_ENABLE, varies (pretty much all of them even for bitfields) */
      MR(0x19C),	/* IA32_THERM_STATUS, all except thread */
      MR(0x19B),	/* MSR_THERM2_CTL, unique and shared */
      MR(0x1A2),	/* MSR_TEMPERATURE_TARGET, varies (pretty much all of them) */
      MR(0x309),	/* IA32_FIXED_CTR0, unique, core and thread */
      MR(0x30A),	/* IA32_FIXED_CTR1, unique, core and thread */
      MR(0x30B),	/* IA32_FIXED_CTR2, unique, core and thread */
      MR(0x38D),	/* IA32_FIXED_CTR_CTRL, Unique, shared, core, and thread */
      MR(0xC1),	/* IA32_PMC0, thread, core and unique */
      MR(0xC2),	/* IA32_PMC1 */
      MR(0xC3),	/* IA32_PMC2 */
      MR(0xC4),	/* IA32_PMC3 */
#if 0
      MR(0xC5),	/* IA32_PMC4 */
      MR(0xC6),	/* IA32_PMC5 */
      MR(0xC7),	/* IA32_PMC6 */
      MR(0xC8),	/* IA32_PMC7 */
#endif
      MR(0x345),	/* IA32_PERF_CAPABILITIES, unique, shared, core, and thread */
      MR(0x38E),	/* IA32_PERF_GLOBAL_STATUS, thread, core, and unique */
      MR(0x38F),	/* IA32_PERF_GLOBAL_CTRL, thread, core, and unique */
      MR(0x390),	/* IA32_PERF_GLOBAL_OVF_CTRL, thread, core, and unique */
#if 0
      MR(0x391),	/* MSR_UNCORE_PERF_GLOBAL_CTRL, package */
      MR(0x392),	/* MSR_UNCORE_PERF_GLOBAL_CTRL, package */
#endif
      MR(0x3F1),	/* MSR_PEBS_ENABLE, thread, shared, unique, core */
      MR(0x606),	/* MSR_RAPL_POWER_UNIT, package */
      MR(0x610),	/* MSR_PKG_POWER_LIMIT, package */
      MR(0x611),	/* MSR_PKG_ENERGY_STATUS, package */
#if 0
      MR(0x613),	/* MSR_PKG_PERF_STATUS, package */
#endif
      MR(0x614),	/* MSR_PKG_POWER_INFO, package */
      MR(0x618),	/* MSR_DRAM_POWER_LIMIT, package */
      MR(0x619),	/* MSR_DRAM_ENERGY_STATUS, package */
      MR(0x61B),	/* MSR_DRAM_PERF_STATUS, package */
      MR(0x61C),	/* MSR_DRAM_POWER_INFO, package */
      MR(0x638),	/* MSR_PP0_POWER_LIMIT, package */
      MR(0x639),	/* MSR_PP0_ENERGY_STATUS, package */
#if 0
      MR(0x63A),	/* MSR_PP0_POLICY, package */
      MR(0x63B),	/* MSR_PP0_PERF_STATUS, package */
      MR(0x640),	/* MSR_PP1_POWER_LIMIT, package */
      MR(0x641),	/* MSR_PP1_ENERGY_STATUS, package */
      MR(0x642),	/* MSR_PP1_POLICY, package */
      MR(0x64C),	/* MSR_TURBO_ACTIVATION_RATIO, package */
      MR(0x66E),	/* MSR_PKG_POWER_INFO, package */
      MR(0x690),	/* MSR_CORE_PERF_LIMIT_REASONS, package */
      MR(0x6B0),	/* MSR_GRAPHICS_PERF_LIMIT_REASONS, package */
      MR(0x6B1),	/* MSR_RING_PERF_LIMIT_REASONS, package */
#endif
      MR(0xE7)	/* IA32_MPERF, varies (unique to thread) */
    }
  },
  MRCOPY(1), MRCOPY(2), MRCOPY(3), MRCOPY(4), MRCOPY(5), MRCOPY(6), MRCOPY(7),
  MRCOPY(8), MRCOPY(9), MRCOPY(10), MRCOPY(10), MRCOPY(11), MRCOPY(12),
  MRCOPY(13), MRCOPY(14), MRCOPY(15), MRCOPY(16), MRCOPY(17), MRCOPY(18),
  MRCOPY(19), MRCOPY(20), MRCOPY(21), MRCOPY(22), MRCOPY(23), MRCOPY(24),
  MRCOPY(25), MRCOPY(26), MRCOPY(27), MRCOPY(28), MRCOPY(29), MRCOPY(30),
  MRCOPY(31), MRCOPY(32), MRCOPY(33), MRCOPY(34), MRCOPY(35), MRCOPY(36),
  MRCOPY(37), MRCOPY(38), MRCOPY(39), MRCOPY(40), MRCOPY(41), MRCOPY(42)
};

void check_bundle(struct msr_bundle_desc *bd)
{
	struct msr_cpu_ops *mco;
	struct msr_op *op;

	for (mco = bd->bundle; mco < bd->bundle + bd->n_msr_bundles; ++mco) {
		for (op = mco->ops; op < &mco->ops[mco->n_ops]; ++op) {
			if (!op->errno)
				continue;
			fprintf(stderr, "***CHECK***: CPU %d, %s %x, Err %d\n",
				mco->cpu, op->isread ?  "RDMSR" : "WRMSR",
							op->msr, op->errno);
		}
	}
}

void print_bundle_err(struct msr_bundle_desc *bd)
{
	struct msr_cpu_ops *mco;
	struct msr_op *op;

	for (mco = bd->bundle; mco < bd->bundle + bd->n_msr_bundles; ++mco) {
		for (op = mco->ops; op < &mco->ops[mco->n_ops]; ++op) {
			if (!op->errno)
				continue;
			fprintf(stderr, "CPU %d, %s %x, Err %d\n",
				mco->cpu, op->isread ?  "RDMSR" : "WRMSR",
							op->msr, op->errno);
		}
	}
}

void print_bundle_info(struct msr_bundle_desc *bd)
{
	struct msr_cpu_ops *mco;
	struct msr_op *op;

	for (mco = bd->bundle; mco < bd->bundle + bd->n_msr_bundles; ++mco) {
		for (op = mco->ops; op < &mco->ops[mco->n_ops]; ++op) {
			printf("CPU %d: %s %x Data %lld\n",
				  mco->cpu, op->isread ? "RDMSR" : "WRMSR",
							op->msr, op->d.d64);
		}
	}
}

unsigned long long get_tick()
{
	unsigned long long rval = 0;

	if (pread(fd[0], &rval, sizeof(rval), 0xE7) < 0) {
		perror("pread");
		_exit(1);
	}
	return rval;
}

int main()
{
	const int lcount = 10000;
	unsigned long long start_tick, end_tick;
	char fname[100];
	struct msr_bundle_desc bd;
	int i, j;
	struct msr_cpu_ops *mco;
	struct msr_op *op;


	bd.n_msr_bundles = sizeof(bundle) / sizeof(bundle[0]);
	bd.bundle = bundle;

	if ((batchfd = open("/dev/cpu/msr_batch", O_RDWR)) < 0) {
		perror("/dev/cpu/msr_batch");
		_exit(1);
	}

	for (i = 0; i < bd.n_msr_bundles; ++i) {
		sprintf(fname, "/dev/cpu/%d/msr_safe_beta", i);
		if ((fd[i] = open(fname, O_RDWR)) < 0) {
			perror(fname);
			_exit(1);
		}
	}

	start_tick = get_tick();
	for (j = 0; j < lcount; ++j) {
		if (ioctl(batchfd, X86_IOC_MSR_BATCH, &bd) < 0) {
			perror("Ioctl failed");
			print_bundle_err(&bd);
			_exit(1);
		}
	}
	end_tick = get_tick();
	check_bundle(&bd);
#if 0
	print_bundle_info(&bd);
#endif

	printf("Batch took %llu ticks\n", (end_tick - start_tick) / lcount);

	start_tick = get_tick();
	for (j = 0; j < lcount; ++j) {
		for (mco = bd.bundle; mco < bd.bundle + bd.n_msr_bundles; ++mco) {
			for (op = mco->ops; op < &mco->ops[mco->n_ops]; ++op) {
				if (pread(fd[mco->cpu], &op->d.d64, sizeof(op->d.d64), op->msr) < 0) {
					perror("pread");
					_exit(1);
				}
			}
		}
	}
	end_tick = get_tick();

	printf("Serial took %llu ticks\n", (end_tick - start_tick) / lcount);

	for (i = 0; i < bd.n_msr_bundles; ++i)
		close(fd[i]);

	close(batchfd);

	return 0;
}
