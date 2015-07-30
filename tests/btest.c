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

#define NCPUS		24
#define BATCH_TABLE	batch24_ops

#define MR(__CPU__, __m__) {.err=0, .cpu = __CPU__, .msr=__m__, .msrdata=0, .isrdmsr=1, .wmask=0}

#define THR(__CPU__) \
	MR(__CPU__,  0x10), MR(__CPU__,  0xE7), MR(__CPU__,  0xE8), MR(__CPU__, 0x1A0), MR(__CPU__, 0x19C),\
MR(__CPU__, 0x19B), MR(__CPU__, 0x1A2), MR(__CPU__, 0x309), MR(__CPU__, 0x30A), MR(__CPU__, 0x38D)

#define CORE(__CPU__) /* TBD */

#define PKG(__CPU__) \
	THR(__CPU__), MR(__CPU__, 0x611), MR(__CPU__, 0x61B), MR(__CPU__, 0x619), MR(__CPU__, 0x638), MR(__CPU__, 0x639)

struct msr_batch_op batch1_ops[] =
{
	PKG(12), THR(12), THR(12), THR(12), THR(12), THR(12), THR(12), THR(12), THR(12), THR(12), THR(12), THR(12),
	PKG(12), THR(12), THR(12), THR(12), THR(12), THR(12), THR(12), THR(12), THR(12), THR(12), THR(12), THR(12)
};
struct msr_batch_op batch24_ops[] =
{
	PKG(0),  THR(1),  THR(2),  THR(3),  THR(4),  THR(5),  THR(6),  THR(7),  THR(8),  THR(9),  THR(10), THR(11),
	PKG(12), THR(13), THR(14), THR(15), THR(16), THR(17), THR(18), THR(19), THR(20), THR(21), THR(22), THR(23)
};
struct msr_batch_op batch48_ops[] =
{
	PKG(0),  THR(1),  THR(2),  THR(3),  THR(4),  THR(5),  THR(6),  THR(7),  THR(8),  THR(9),  THR(10), THR(11),
	PKG(12), THR(13), THR(14), THR(15), THR(16), THR(17), THR(18), THR(19), THR(20), THR(21), THR(22), THR(23),
	THR(24), THR(25), THR(26), THR(27), THR(28), THR(29), THR(30), THR(31), THR(32), THR(33), THR(34), THR(35),
	THR(36), THR(37), THR(38), THR(39), THR(40), THR(41), THR(42), THR(43), THR(44), THR(45), THR(46), THR(47)
};
struct msr_batch_op batch72_ops[] =
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

void check_array(struct msr_batch_array *ba)
{
	struct msr_batch_op *op;

	for (op = ba->ops; op < ba->ops + ba->numops; ++op) {
		if (!op->err)
			continue;
		fprintf(stderr, "***CHECK***: CPU %d, RDMSR %x, Err %d\n",
				op->cpu, op->msr, op->err);
	}
}

void print_array_err(struct msr_batch_array *ba)
{
	struct msr_batch_op *op;

	for (op = ba->ops; op < ba->ops + ba->numops; ++op) {
		if (!op->err)
			continue;
		fprintf(stderr, "CPU %d, %s %x, Err %d 0x%llx\n",
				op->cpu, op->isrdmsr ? "RDMSR" : "WRMSR", op->msr, op->err, op->wmask);
	}
}

void print_array_info(struct msr_batch_array *ba)
{
	struct msr_batch_op *op;

	for (op = ba->ops; op < ba->ops + ba->numops; ++op) {
		printf("CPU %d: %s %x Data 0x%016llX\n",
				op->cpu, op->isrdmsr ? "RDMSR" : "WRMSR", op->msr, op->msrdata);
	}
}

void run_rwtest(int *fd, int batchfd)
{
	struct msr_batch_op ops[3];
	struct msr_batch_array ba;

	ops[0].err = 0;
	ops[0].cpu = 0;
	ops[0].msr = 0x1A0;
	ops[0].msrdata = 0;
	ops[0].isrdmsr = 1;
	ops[0].wmask = 0;

	ops[1].err = 0;
	ops[1].cpu = 0;
	ops[1].msr = 0x1A0;
	ops[1].msrdata = 0x9;
	ops[1].isrdmsr = 0;
	ops[1].wmask = 0;

	ops[2].err = 0;
	ops[2].cpu = 0;
	ops[2].msr = 0x1A0;
	ops[2].msrdata = 0;
	ops[2].isrdmsr = 1;
	ops[2].wmask = 0;

	ba.numops = sizeof(ops) / sizeof(ops[0]);
	ba.ops = ops;

	if (ioctl(batchfd, X86_IOC_MSR_BATCH, &ba) < 0) {
		perror("Ioctl failed");
		print_array_err(&ba);
		_exit(1);
	}
	print_array_err(&ba);
	print_array_info(&ba);
}


unsigned long long get_tick(int *fd)
{
	unsigned long long rval = 0;

	if (pread(fd[0], &rval, sizeof(rval), 0x10) < 0) {
		perror("get_tick(): pread");
		_exit(1);
	}
	return rval;
}

void run_batch(int *fd, int batchfd, struct msr_batch_array *oa, const int lcount, double *ticks_per_op, double *us_per_op)
{
	unsigned long long start_tick, ticks;
	int j;
	struct timeval stime, etime;
	struct timezone tz;

	if (gettimeofday(&stime, &tz) < 0) {
		perror("gettimeofday");
		_exit(2);
	}
	start_tick = get_tick(fd);
	for (j = 0; j < lcount; ++j) {
		if (ioctl(batchfd, X86_IOC_MSR_BATCH, oa) < 0) {
			perror("Ioctl failed");
			print_array_err(oa);
			_exit(1);
		}
	}
	ticks = get_tick(fd) - start_tick;
	if (gettimeofday(&etime, &tz) < 0) {
		perror("gettimeofday");
		_exit(2);
	}
	*ticks_per_op = (double)((double)ticks / (double)lcount);
	*us_per_op = (double)((((etime.tv_sec - stime.tv_sec) * 1000000) + etime.tv_usec - stime.tv_usec) / (double)lcount);
}

void run_sequential(int *fd, struct msr_batch_array *oa, const int lcount, double *ticks_per_op, double *us_per_op)
	
{
	unsigned long long start_tick, ticks;
	int j;
	struct timeval stime, etime;
	struct timezone tz;
	struct msr_batch_op *op;

	if (gettimeofday(&stime, &tz) < 0) {
		perror("gettimeofday");
		_exit(2);
	}
	start_tick = get_tick(fd);
	for (j = 0; j < lcount; ++j) {
		for (op = oa->ops; op < oa->ops + oa->numops; ++op) {
			if (pread(fd[op->cpu], &op->msrdata, sizeof(op->msrdata), op->msr) < 0) {
				perror("pread");
				_exit(1);
			}
		}
	}
	ticks = get_tick(fd) - start_tick;
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

	if ((cpuset = CPU_ALLOC(NCPUS)) == NULL) {
		perror("CPU_ALLOC");
		_exit(1);
	}

	size = CPU_ALLOC_SIZE(NCPUS);

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

void run_test(int *fd, int batchfd, struct msr_batch_array *oa, int pass, int cpu)
{
	static double bat_op_ticks_tot, seq_op_ticks_tot, bat_op_usec_tot, seq_op_usec_tot;
	static int passes;
	const int lcount = 10000;
	double bat_op_ticks = 0, seq_op_ticks = 0, bat_op_usec = 0, seq_op_usec = 0;
	double speedup;

	if (pass == 1) {
		bat_op_ticks_tot = 0;
		seq_op_ticks_tot = 0;
		bat_op_usec_tot = 0;
		seq_op_usec_tot = 0;
		passes = 0;
		printf("========================================================\n");
		printf("||                %3d msr operations                  ||\n", oa->numops);
		printf("========================================================\n");
		printf("Pass |CPU |Batch Kop/s |Sequential Kop/s |Batch Speedup|\n");
		printf("-----+----+------------+-----------------+-------------+\n");
	}

	if (pass == -1) {
		bat_op_ticks_tot /= passes;
		seq_op_ticks_tot /= passes;
		bat_op_usec_tot /= passes;
		seq_op_usec_tot /= passes;
		speedup = (1000.0/bat_op_usec_tot) / (1000.0/seq_op_usec_tot);

		printf("-----+----+------------+-----------------+-------------\n");
		printf("AVG  |NA  |%11.1f |%15.1f  |%15.1f\n",
			1000.0/bat_op_usec_tot, 1000.0/seq_op_usec_tot, speedup);
		printf("-----+----+------------+-----------------+-------------\n\n");
		return;
	}

	set_affinity(cpu);

	run_batch(fd, batchfd, oa, lcount, &bat_op_ticks, &bat_op_usec);
	run_sequential(fd, oa, lcount, &seq_op_ticks, &seq_op_usec);
	bat_op_ticks_tot += bat_op_ticks;
	seq_op_ticks_tot += seq_op_ticks;
	bat_op_usec_tot += bat_op_usec;
	seq_op_usec_tot += seq_op_usec;
	++passes;

	speedup = (1000.0/bat_op_usec) / (1000.0/seq_op_usec);

	printf("%4d |%3u |%11.1f |%15.1f  |%13.1f\n",
		pass, sched_getcpu(), 1000.0/bat_op_usec, 1000.0/seq_op_usec, speedup);
}

int main()
{
	char fname[100];
	int i;
	struct msr_batch_array barray, bsingle;
	int batchfd;
	int fd[100];

	if ((batchfd = open("/dev/cpu/msr_batch", O_RDWR)) < 0) {
		perror("/dev/cpu/msr_batch");
		_exit(1);
	}

	for (i = 0; i < NCPUS; ++i) {
		sprintf(fname, "/dev/cpu/%d/msr", i);
		if ((fd[i] = open(fname, O_RDWR)) < 0) {
			perror(fname);
			_exit(1);
		}
	}
	run_rwtest(fd, batchfd);

	bsingle.numops = sizeof(batch1_ops) / sizeof(batch1_ops[0]);
	bsingle.ops = batch1_ops;

	run_test(fd, batchfd, &bsingle, 1, 12);
	run_test(fd, batchfd, &bsingle, -1, 0);

	run_test(fd, batchfd, &bsingle, 1, 16);
	run_test(fd, batchfd, &bsingle, -1, 0);

	barray.numops = sizeof(BATCH_TABLE) / sizeof(BATCH_TABLE[0]);
	barray.ops = BATCH_TABLE;

	for (i = 0; i < NCPUS; i++)
		run_test(fd, batchfd, &barray, i+1, i);
	run_test(fd, batchfd, &barray, -1, 0);

	for (i = 0; i < NCPUS; ++i)
		close(fd[i]);

	close(batchfd);

	return 0;
}
