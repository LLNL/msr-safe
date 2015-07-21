// Program to test out new ioctl interface to /dev/cpu/0/msr_safe
#define _GNU_SOURCE
#include <linux/types.h>
#include <sched.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <linux/ioctl.h>
#include <unistd.h>

#ifdef DOCUMENTATION
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
#endif // For now...

#define MAX_POSSIBLE_CPUS	100
int batchfd = 0;
int fd[MAX_POSSIBLE_CPUS];
int ncpu = 0;

void open_msr_files()
{
	char fname[MAX_POSSIBLE_CPUS];
	struct stat sbuf;
	int i;

	for (i = 0; i < MAX_POSSIBLE_CPUS; ++i) {
		sprintf(fname, "/dev/cpu/%d/msr", i);
		if (stat(fname, &sbuf)) {
			fd[i] = 0;
			continue;
		}

		if ((fd[i] = open(fname, O_RDWR)) < 0) {
			perror(fname);
			_exit(1);
		}
	}
}

unsigned long long get_tick()
{
	unsigned long long rval = 0;
	unsigned long long foo1 = 0x684e0001483f7;
	unsigned long long foo =  0x684e000148410;

	if (pread(fd[0], &rval, sizeof(rval), 0x610) < 0) {
		perror("get_tick");
		_exit(1);
	}
	printf("MSR 0x610: is %llx\n", rval);

	if (pwrite(fd[0], &foo1, sizeof(rval), 0x610) < 0) {
		perror("get_tick");
		_exit(1);
	}
	if (pread(fd[0], &rval, sizeof(rval), 0x610) < 0) {
		perror("get_tick");
		_exit(1);
	}
	printf("MSR 0x610: is %llx\n", rval);

	if (pwrite(fd[0], &foo, sizeof(rval), 0x610) < 0) {
		perror("get_tick");
		_exit(1);
	}
	if (pread(fd[0], &rval, sizeof(rval), 0x610) < 0) {
		perror("get_tick");
		_exit(1);
	}
	printf("MSR 0x610: is %llx\n", rval);
	return rval;
}

int main()
{
	open_msr_files();
	get_tick();
	return 0;
}
