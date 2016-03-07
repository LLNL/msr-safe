#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

char* progname = NULL;
char* progversion = "0.0.1";

void Usage()
{
	fprintf(stderr, "Usage:\n %s ", progname);
	fprintf(stderr,
		" -msr # (default is 0x0)\n"
		"          -data # (default is 0)\n"
		"          -i # (default is 10000000)\n"
		"          -src # (Source CPU, default is 0)\n"
		"          -target # (Target CPU, default is 0)\n"
		"\n\n");
	_exit(-1);
}

int main(int ac, char** av)
{
	unsigned long long msrdata = 0x0LL;
	unsigned int msr = 0x0;		// No-op
	unsigned int iterations = 10000000;
	unsigned int mycpu = 0;
	unsigned int msrcpu = 0;
	int argidx = 0;
	unsigned long long data;
	char filename[80];
	int fd;

	/* Process Command line arguments */
	progname = av[argidx++];

	for (ac = ac - 1; ac > 0; ) {
		char *arg = av[argidx++];
		char *arg2 = av[argidx++];
		if (strcmp(arg, "-msr") == 0) {
			msr = strtoul(arg2, NULL, 0);
		} else if (strcmp(arg, "-data") == 0) {
			msrdata = strtoull(arg2, NULL, 0);
		} else if (strcmp(arg, "-i") == 0) {
			iterations = strtoul(arg2, NULL, 0);
		} else if (strcmp(arg, "-src") == 0) {
			mycpu = strtoul(arg2, NULL, 0);
		} else if (strcmp(arg, "-target") == 0) {
			msrcpu = strtoul(arg2, NULL, 0);
		} else {
			fprintf(stderr, "Invalid argument %s\n", arg);
			Usage();
		}
	}

	sprintf(filename, "/dev/cpu/%d/msr", msrcpu);

	printf("Opening %s\n", filename);
	if ((fd = open(filename, O_RDWR)) < 0) {
		perror(filename);
		_exit(-1);
	}

	if (msrdata == 0LL) {	/* Assume read */
		printf("Reading from MSR 0x%x, %d times\n", msr, iterations);
		for (int i = 0; i < iterations; i++) {
			if (pread(fd, &data, sizeof(data), msr) < 0) {
				perror("pread");
				_exit(-2);
			}
		}
	}
	else {
		printf("Writing 0x%llx to MSR 0x%x, %d times\n", msrdata, msr, iterations);
	}

	(void)close(fd);
	return 0;
}
