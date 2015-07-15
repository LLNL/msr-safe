// Program to test out new ioctl interface to /dev/cpu/0/msr_safe
#define _GNU_SOURCE
#include <linux/types.h>
#include <sched.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <linux/ioctl.h>
#include <unistd.h>
#include "../msr.h"

#define MW(m,v) {{ .d64 = v }, .mask=0, .msr=m, .isread=0, .errno=0}
#define MR(m) {{ .d64 = 0 }, .mask=0, .msr=m, .isread=1, .errno=0}
#define PERTHR(__CPU__) \
  { .cpu = __CPU__, .n_ops = 10, { MR(0x10), MR(0xE7), MR(0xE8), MR(0x1A0), MR(0x19C), MR(0x19B), MR(0x1A2), MR(0x309), MR(0x30A), MR(0x38D) } }
#define PERCORE(__CPU__) /* TBD */
#define PERPKG(__CPU__) \
  { .cpu = __CPU__, .n_ops = 15, { MR(0x10), MR(0xE7), MR(0xE8), MR(0x19C), MR(0x1A0), MR(0x19B), MR(0x1A2), MR(0x309), MR(0x30A), MR(0x38D), MR(0x611), MR(0x61B), MR(0x619), MR(0x638), MR(0x639) } }

#ifdef HEREZ
// All Threads
MR(0x10),	// IA32_TIME_STAMP_COUNTER, thread
MR(0xE7),	// IA32_MPERF, varies (unique to thread)
MR(0xE8),	// IA32_APERF, varies (unique to thread)
MR(0x1A0),	// IA32_MISC_ENABLE, varies (pretty much all of them even for bitfields)
MR(0x309),	// IA32_FIXED_CTR0
MR(0x30A),	// IA32_FIXED_CTR1
MR(0x30B),	// IA32_FIXED_CTR2

// All Packages
MR(0x611),	// MSR_PKG_ENERGY_STATUS, package
// MR(0x613),	// MSR_PKG_PERF_STATUS, package
MR(0x639),	// MSR_PP0_ENERGY_STATUS, package
// MR(0x63B),	// MSR_PP0_PERF_STATUS, package
// MR(0x63A),	// MSR_PP0_POLICY, package
// MR(0x641),	// MSR_PP1_ENERGY_STATUS, package
// MR(0x642),	// MSR_PP1_POLICY, package
MR(0x619),	// MSR_DRAM_ENERGY_STATUS, package
MR(0x61B),	// MSR_DRAM_PERF_STATUS, package

// All Cores (TBD)
#endif // For now...

// #define ADA 1
#ifndef ADA
struct msr_cpu_ops bundle[] = 
{
	PERPKG(0),  PERTHR(1),  PERTHR(2),  PERTHR(3),  PERTHR(4),  PERTHR(5),  PERTHR(6),  PERTHR(7),  PERTHR(8),  PERTHR(9),  PERTHR(10), PERTHR(11),
	PERPKG(12), PERTHR(13), PERTHR(14), PERTHR(15), PERTHR(16), PERTHR(17), PERTHR(18), PERTHR(19), PERTHR(20), PERTHR(21), PERTHR(22), PERTHR(23),
	PERTHR(24), PERTHR(25), PERTHR(26), PERTHR(27), PERTHR(28), PERTHR(29), PERTHR(30), PERTHR(31), PERTHR(32), PERTHR(33), PERTHR(34), PERTHR(35),
	PERTHR(36), PERTHR(37), PERTHR(38), PERTHR(39), PERTHR(40), PERTHR(41), PERTHR(42), PERTHR(43), PERTHR(44), PERTHR(45), PERTHR(46), PERTHR(47)
};
#else
struct msr_cpu_ops bundle[] = 
{
	PERPKG(0),  PERTHR(1),  PERTHR(2),  PERTHR(3),  PERTHR(4),  PERTHR(5),  PERTHR(6),  PERTHR(7),  PERTHR(8),  PERTHR(9),  PERTHR(10), PERTHR(11), PERTHR(12), PERTHR(13), PERTHR(14), PERTHR(15), PERTHR(16), PERTHR(17), 
	PERPKG(18), PERTHR(19), PERTHR(20), PERTHR(21), PERTHR(22), PERTHR(23), PERTHR(24), PERTHR(25), PERTHR(26), PERTHR(27), PERTHR(28), PERTHR(29), PERTHR(30), PERTHR(31), PERTHR(32), PERTHR(33), PERTHR(34), PERTHR(35),
	PERTHR(36), PERTHR(37), PERTHR(38), PERTHR(39), PERTHR(40), PERTHR(41), PERTHR(42), PERTHR(43), PERTHR(44), PERTHR(45), PERTHR(46), PERTHR(47), PERTHR(48), PERTHR(49), PERTHR(50), PERTHR(51), PERTHR(52), PERTHR(53),
	PERTHR(54), PERTHR(55), PERTHR(56), PERTHR(57), PERTHR(58), PERTHR(59), PERTHR(60), PERTHR(61), PERTHR(62), PERTHR(63), PERTHR(64), PERTHR(65), PERTHR(66), PERTHR(67), PERTHR(68), PERTHR(69), PERTHR(70), PERTHR(71)
};
#endif //

struct msr_bundle_desc bd;
int batchfd;
int fd[100];

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

void run_test(int pass, int cpu)
{
	const int lcount = 1000;
	unsigned long long start_tick, sequential_ticks, batch_ticks;
	char fname[100];
	int i, j;
	struct msr_cpu_ops *mco;
	struct msr_op *op;
	double factor;
	int mycpu;
	int cpucheck;
	size_t cpusetsz;
	cpu_set_t *cpuset;
	size_t size;

	if ((cpuset = CPU_ALLOC(bd.n_msr_bundles)) == NULL) {
		perror("CPU_ALLOC");
		_exit(1);
	}

	size = CPU_ALLOC_SIZE(bd.n_msr_bundles);

	CPU_ZERO_S(size, cpuset);
	CPU_SET_S(cpu, size, cpuset);

	if (sched_setaffinity(0, size, cpuset) < 0) {
		perror("sched_setaffinity: SKIPPED");
		return;
	}

	if ((mycpu = sched_getcpu()) < 0) {
		perror("GetCPU Failed");
		_exit(2);
	}

	start_tick = get_tick();
	for (j = 0; j < lcount; ++j) {
		if (ioctl(batchfd, X86_IOC_MSR_BATCH, &bd) < 0) {
			perror("Ioctl failed");
			print_bundle_err(&bd);
			_exit(1);
		}
	}
	batch_ticks = get_tick() - start_tick;

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
	sequential_ticks = get_tick() - start_tick;

	if ((cpucheck = sched_getcpu()) < 0) {
		perror("GetCPU Failed");
		_exit(2);
	}

	if (cpucheck != mycpu)
		printf("WARNING: CPU Migration Occurred\n");

	factor = (double)sequential_ticks / (double)batch_ticks;
	printf("Pass %2d: CPU %u: (Serial %llu) / (Batch %llu) =  %f factor\n", 
		pass, cpucheck, sequential_ticks / lcount, batch_ticks / lcount, factor);
	CPU_FREE(cpuset);
}

int main()
{
	char fname[100];
	int i;

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

	for (i = 0; i < bd.n_msr_bundles; ++i)
		run_test(i+1, i);

	for (i = 0; i < bd.n_msr_bundles; ++i)
		close(fd[i]);

	close(batchfd);

	return 0;
}
