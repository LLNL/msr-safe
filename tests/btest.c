// Program to test out new ioctl interface to /dev/cpu/0/msr_safe
#include <linux/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <linux/ioctl.h>
#include <unistd.h>
#include "../msr.h"

struct msr_cpu_ops bundle[] = 
{
  { .cpu = 0, .n_ops = 2, 
    {
      { { .d64 = 0 }, .mask = 0, .msr = 0xe7, .isread = 1, .errno = 0 },  
      { { .d64 = 0 }, .mask = 0, .msr = 0xe7, .isread = 1, .errno = 0 }
    }
  },
  { .cpu = 22, .n_ops = 1, 
    {
      { { .d64 = 0 }, .mask = 0, .msr = 0xe7, .isread = 1, .errno = 0 }
    }
  }
};

int main()
{
	int fd;
	struct msr_bundle_desc bd;
	int i, j;

	if ((fd = open("/dev/cpu/0/msr_safe", O_RDWR)) < 0) {
		perror("Failed to open /dev/cpu/0/msr_safe");
		_exit(1);
	}

	bd.n_msr_bundles = 2;
	bd.bundle = bundle;

	if (ioctl(fd, X86_IOC_MSR_BATCH, &bd) < 0) {
		perror("Ioctl failed");
		for (i = 0; i < bd.n_msr_bundles; ++i) {
			for (j = 0; j < bundle[i].n_ops; ++j) {
				if (bundle[i].ops[j].errno) {
					fprintf(stderr, 
					    "CPU %d, %s %x, Err %d\n", 
						bundle[i].cpu,
						bundle[i].ops[j].isread ? 
							"RDMSR" : "WRMSR",
						bundle[i].ops[j].msr, 
						bundle[i].ops[j].errno);
				}
			}
		}
		_exit(1);
	}
	
	for (i = 0; i < bd.n_msr_bundles; ++i) {
		for (j = 0; j < bundle[i].n_ops; ++j) {
			if (bundle[i].ops[j].errno) {
				fprintf(stderr, "Huh? %s %x, Err %d\n",
				  bundle[i].ops[j].isread ? "RDMSR" : "WRMSR",
				     bundle[i].ops[j].msr, 
					bundle[i].ops[j].errno);
			}
			else {
				printf("%s %x Data %lld\n", 
				  bundle[i].ops[j].isread ? "RDMSR" : "WRMSR",
					bundle[i].ops[j].msr, 
					bundle[i].ops[j].d.d64);
			}
		}
	}
	
	close(fd);
	return 0;
}
