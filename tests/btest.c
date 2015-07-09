// Program to test out new ioctl interface to /dev/cpu/0/msr_safe
#include <linux/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <linux/ioctl.h>
#include <unistd.h>
#include "../msr.h"

int main()
{
	int fd;
	struct msr_bundle_desc bd;
	struct msr_cpu_ops ops[4];
	int i, j;

	if ((fd = open("/dev/cpu/0/msr_safe", O_RDWR)) < 0) {
		perror("Failed to open /dev/cpu/0/msr_safe");
		_exit(1);
	}

	bd.n_msr_bundles = 2;
	ops[0].cpu = 0;
	ops[0].n_ops = 2;
	ops[0].ops[0].msr = 0xe7;
	ops[0].ops[0].errno = 0;
	ops[0].ops[0].d.d64 = 0;
	ops[0].ops[1].msr = 0xe7;
	ops[0].ops[1].errno = 0;
	ops[0].ops[1].d.d64 = 0;

	ops[1].cpu = 22;
	ops[1].n_ops = 1;
	ops[1].ops[0].msr = 0xe7;
	ops[1].ops[0].errno = 0;
	ops[1].ops[0].d.d64 = 0;

	bd.bundle = ops;

	if (ioctl(fd, X86_IOC_RDMSR_BATCH, &bd) < 0) {
		perror("Ioctl failed");
		for (i = 0; i < bd.n_msr_bundles; ++i) {
			for (j = 0; j < ops[i].n_ops; ++j) {
				if (ops[i].ops[j].errno) {
					fprintf(stderr, 
					    "CPU %d, MSR %x, Err %d\n", 
						ops[i].cpu,
						ops[i].ops[j].msr, 
						ops[i].ops[j].errno);
				}
			}
		}
		_exit(1);
	}
	
	for (i = 0; i < bd.n_msr_bundles; ++i) {
		for (j = 0; j < ops[i].n_ops; ++j) {
			if (ops[i].ops[j].errno) {
				fprintf(stderr, "Huh? MSR %x, Err %d\n",
				     ops[i].ops[j].msr, ops[i].ops[j].errno);
			}
			else {
				printf("MSR %x Data %lld\n", 
					ops[i].ops[j].msr, ops[i].ops[j].d.d64);
			}
		}
	}
	
	close(fd);
	return 0;
}
