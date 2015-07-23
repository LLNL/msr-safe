/* Program to test out new ioctl interface to /dev/cpu/msr_batch */
#define _GNU_SOURCE
#include <linux/types.h>
#include <sched.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <linux/ioctl.h>
#include <unistd.h>
#include <sys/time.h>
#include "../msr.h"

#define MR(__CPU__, __m__) {.err=0, .cpu = __CPU__, .msr=__m__, .msrdata=0}

#define THR(__CPU__) \
	MR(__CPU__, 0x10), /* IA32_TIME_STAMP_COUNTER, thread */ \
	MR(__CPU__, 0xE7), /* IA32_MPERF, varies (unique to thread) */ \
	MR(__CPU__, 0xE8), /* IA32_APERF, varies (unique to thread) */ \
	MR(__CPU__, 0x1A0), /* IA32_MISC_ENABLE, varies (pretty much all of them even for bitfields) */ \
	MR(__CPU__, 0x19C), \
	MR(__CPU__, 0x19B), \
	MR(__CPU__, 0x1A2), \
	MR(__CPU__, 0x309), /* IA32_FIXED_CTR0 */ \
	MR(__CPU__, 0x30A), /* IA32_FIXED_CTR1 */ \
	MR(__CPU__, 0x38D) /* IA32_FIXED_CTR2 */

#define CORE(__CPU__) /* TBD */

#define PKG(__CPU__) \
	MR(__CPU__, 0x10), \
	MR(__CPU__, 0xE7), \
	MR(__CPU__, 0xE8), \
	MR(__CPU__, 0x19C), \
	MR(__CPU__, 0x1A0), \
	MR(__CPU__, 0x19B), \
	MR(__CPU__, 0x1A2), \
	MR(__CPU__, 0x309), \
	MR(__CPU__, 0x30A), \
	MR(__CPU__, 0x38D), \
	MR(__CPU__, 0x611), /* MSR_PKG_ENERGY_STATUS, package */ \
	/* MR(0x613),	/* MSR_PKG_PERF_STATUS, package */ \
	MR(__CPU__, 0x61B), /* MSR_DRAM_PERF_STATUS, package */ \
	MR(__CPU__, 0x619), /* MSR_DRAM_ENERGY_STATUS, package */ \
	MR(__CPU__, 0x638), \
	MR(__CPU__, 0x639) /* MSR_PP0_ENERGY_STATUS, package */
/* MR(0x63A),	/* MSR_PP0_POLICY, package */
/* MR(0x63B),	/* MSR_PP0_PERF_STATUS, package */
/* MR(0x641),	/* MSR_PP1_ENERGY_STATUS, package */
/* MR(0x642),	/* MSR_PP1_POLICY, package */

/* #define ADA 1 */
#ifndef ADA
#ifdef MULT_THREAD
int ncpus = 48;
struct msr_batch_rdmsr_op batch_ops[] =
{
	PKG(0),  THR(1),  THR(2),  THR(3),  THR(4),  THR(5),  THR(6),  THR(7),  THR(8),  THR(9),  THR(10), THR(11),
	PKG(12), THR(13), THR(14), THR(15), THR(16), THR(17), THR(18), THR(19), THR(20), THR(21), THR(22), THR(23),
	THR(24), THR(25), THR(26), THR(27), THR(28), THR(29), THR(30), THR(31), THR(32), THR(33), THR(34), THR(35),
	THR(36), THR(37), THR(38), THR(39), THR(40), THR(41), THR(42), THR(43), THR(44), THR(45), THR(46), THR(47)
};
#else
int ncpus = 24;
struct msr_batch_rdmsr_op batch_ops[] =
{
	PKG(0),  THR(1),  THR(2),  THR(3),  THR(4),  THR(5),  THR(6),  THR(7),  THR(8),  THR(9),  THR(10), THR(11),
	PKG(12), THR(13), THR(14), THR(15), THR(16), THR(17), THR(18), THR(19), THR(20), THR(21), THR(22), THR(23)
};
#endif // MULT_THREAD
#else
int ncpus = 72;
struct msr_batch_rdmsr_op batch_ops[] =
{
	PKG(0),  THR(1),  THR(2),  THR(3),  THR(4),  THR(5),  THR(6),  THR(7),  THR(8),  
	THR(9),  THR(10), THR(11), THR(12), THR(13), THR(14), THR(15), THR(16), THR(17),
	PKG(18), THR(19), THR(20), THR(21), THR(22), THR(23), THR(24), THR(25), THR(26), 
	THR(27), THR(28), THR(29), THR(30), THR(31), THR(32), THR(33), THR(34), THR(35),
	THR(36), THR(37), THR(38), THR(39), THR(40), THR(41), THR(42), THR(43), THR(44), 
	THR(45), THR(46), THR(47), THR(48), THR(49), THR(50), THR(51), THR(52), THR(53),
	THR(54), THR(55), THR(56), THR(57), THR(58), THR(59), THR(60), THR(61), THR(62), 
	THR(63), THR(64), THR(65), THR(66), THR(67), THR(68), THR(69), THR(70), THR(71)
};
#endif

struct msr_batch_rdmsr_array barray;
int batchfd;
int fd[100];

void check_array(struct msr_batch_rdmsr_array *ba)
{
	struct msr_batch_rdmsr_op *op;

	for (op = ba->ops; op < ba->ops + ba->numops; ++op) {
		if (!op->err)
			continue;
		fprintf(stderr, "***CHECK***: CPU %d, RDMSR %x, Err %d\n",
			op->cpu, op->msr, op->err);
	}
}

void print_array_err(struct msr_batch_rdmsr_array *ba)
{
	struct msr_batch_rdmsr_op *op;

	for (op = ba->ops; op < ba->ops + ba->numops; ++op) {
		if (!op->err)
			continue;
		fprintf(stderr, "CPU %d, RDMSR %x, Err %d\n",
			op->cpu, op->msr, op->err);
	}
}

void print_array_info(struct msr_batch_rdmsr_array *ba)
{
	struct msr_batch_rdmsr_op *op;

	for (op = ba->ops; op < ba->ops + ba->numops; ++op) {
		printf("CPU %d: %s %x Data %lld\n",
		  op->cpu, "RDMSR", op->msr, op->msrdata);
	}
}

unsigned long long get_tick()
{
	unsigned long long rval = 0;

	if (pread(fd[0], &rval, sizeof(rval), 0x10) < 0) {
		perror("get_tick(): pread");
		_exit(1);
	}
	return rval;
}

void run_batch(const int lcount, double *ticks_per_op, double *us_per_op)
{
	unsigned long long start_tick, ticks;
	int j;
	struct timeval stime, etime;
	struct timezone tz;

	if (gettimeofday(&stime, &tz) < 0) {
		perror("gettimeofday");
		_exit(2);
	}
	start_tick = get_tick();
	for (j = 0; j < lcount; ++j) {
		if (ioctl(batchfd, X86_IOC_MSR_RDMSR_BATCH, &barray) < 0) {
			perror("Ioctl failed");
			print_array_err(&barray);
			_exit(1);
		}
	}
	ticks = get_tick() - start_tick;
	if (gettimeofday(&etime, &tz) < 0) {
		perror("gettimeofday");
		_exit(2);
	}
	*ticks_per_op = (double)((double)ticks / (double)lcount);
	*us_per_op = (double)((((etime.tv_sec - stime.tv_sec) * 1000000) + etime.tv_usec - stime.tv_usec) / (double)lcount);
}

void run_sequential(const int lcount, double *ticks_per_op, double *us_per_op)
{
	unsigned long long start_tick, ticks;
	int j;
	struct timeval stime, etime;
	struct timezone tz;
	struct msr_batch_rdmsr_op *op;

	if (gettimeofday(&stime, &tz) < 0) {
		perror("gettimeofday");
		_exit(2);
	}
	start_tick = get_tick();
	for (j = 0; j < lcount; ++j) {
		for (op = barray.ops; op < barray.ops + barray.numops; ++op) {
			if (pread(fd[op->cpu], &op->msrdata, sizeof(op->msrdata), op->msr) < 0) {
				perror("pread");
				_exit(1);
			}
		}
	}
	ticks = get_tick() - start_tick;
	if (gettimeofday(&etime, &tz) < 0) {
		perror("gettimeofday");
		_exit(2);
	}
	*ticks_per_op = (double)((double)ticks / (double)lcount);
	*us_per_op = (double)((((etime.tv_sec - stime.tv_sec) * 1000000) + etime.tv_usec - stime.tv_usec) / (double)lcount);
}

void set_affinity(int cpu)
{
	int mycpu;
	int cpucheck;
	size_t cpusetsz;
	cpu_set_t *cpuset;
	size_t size;

	if ((cpuset = CPU_ALLOC(ncpus)) == NULL) {
		perror("CPU_ALLOC");
		_exit(1);
	}

	size = CPU_ALLOC_SIZE(ncpus);

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

	if (mycpu != cpu) {
		fprintf(stderr, "Did not bind to processor %d\n", cpu);
		_exit(2);
	}
}

void run_test(int pass, int cpu)
{
	static double bat_op_ticks_tot = 0, seq_op_ticks_tot = 0, bat_op_usec_tot = 0, seq_op_usec_tot = 0;
	static int passes = 0;
	const int lcount = 10000;
	double bat_op_ticks = 0, seq_op_ticks = 0, bat_op_usec = 0, seq_op_usec = 0;
	double speedup;

	if (pass == -1) {
		speedup = seq_op_usec_tot / bat_op_usec_tot;
		bat_op_ticks_tot /= passes;
		seq_op_ticks_tot /= passes;
		bat_op_usec_tot /= passes;
		seq_op_usec_tot /= passes;

		printf("-----+----+------------+-----------------+-------------\n");
		printf("AVG  |NA  |%11.3f |%15.3f  |%13.3f\n",
			bat_op_usec_tot, seq_op_usec_tot, speedup);
		printf("-----+----+------------+-----------------+-------------\n");
		return;
	}


	set_affinity(cpu);

	run_batch(lcount, &bat_op_ticks, &bat_op_usec);
	run_sequential(lcount, &seq_op_ticks, &seq_op_usec);
	bat_op_ticks_tot += bat_op_ticks;
	seq_op_ticks_tot += seq_op_ticks;
	bat_op_usec_tot += bat_op_usec;
	seq_op_usec_tot += seq_op_usec;
	++passes;

	speedup = seq_op_usec / bat_op_usec;

	if (pass == 1) {
		printf("Pass  CPU  Batch us/op  Sequential us/op  Batch Speedup\n");
		printf("-----+----+------------+-----------------+-------------\n");
	}

	printf("%4d |%3u |%11.3f |%15.3f  |%13.3f\n",
		pass, sched_getcpu(), bat_op_usec, seq_op_usec, speedup);
}

int main()
{
	char fname[100];
	int i;

	barray.numops = sizeof(batch_ops) / sizeof(batch_ops[0]);
	barray.ops = batch_ops;

	if ((batchfd = open("/dev/cpu/msr_batch", O_RDWR)) < 0) {
		perror("/dev/cpu/msr_batch");
		_exit(1);
	}

	for (i = 0; i < ncpus; ++i) {
		sprintf(fname, "/dev/cpu/%d/msr", i);
		if ((fd[i] = open(fname, O_RDWR)) < 0) {
			perror(fname);
			_exit(1);
		}
	}

	for (i = 0; i < ncpus; i++)
		run_test(i+1, i);
	run_test(-1, 0);

	for (i = 0; i < ncpus; ++i)
		close(fd[i]);

	close(batchfd);

	return 0;
}
