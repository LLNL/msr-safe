#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

char* progname = "msrkotest";
char* progversion = "0.0.1";
unsigned long long msrdata = 0x0LL;
unsigned long iterations = 1;
unsigned int msr = 0x0;		// No-op
unsigned int mycpu = 0;
unsigned int msrcpu = 0;

int main(int ac, char** av)
{
	unsigned long long data;
	char filename[80];
	int fd;

	sprintf(filename, "/dev/cpu/%d/msr", msrcpu);

	if ((fd = open(filename, O_RDWR)) < 0) {
		perror(filename);
		_exit(-1);
	}

	if (msrdata == 0LL) {	/* Assume read */
		for (int i = 0; i < iterations; i++) {
			if (pread(fd, &data, sizeof(data), msr) < 0) {
				perror("pread");
				_exit(-2);
			}
		}
	}
	else {
	}


	(void)close(fd);
	return 0;
}
