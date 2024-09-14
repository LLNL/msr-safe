/* This example assumes the user has the following permissions:
 *
 * write        /dev/cpu/msr_allowlist
 * read/write   /dev/cpu/<cpu_number>/msr_safe
 * read         /dev/cpu/msr_batch
 *
 * Typically, only the administrator will have write permissions
 * on the allowlist.
 *
 * Production code should have more robust error handling than
 * what is shown here.
 *
 * This example should be able to run successfully on an x86
 * processor from the past ten years or so.
 *
 */


#include <stdio.h>      // printf(3)
#include <assert.h>     // assert(3)
#include <fcntl.h>      // open(2)
#include <unistd.h>     // write(2), pwrite(2), pread(2)
#include <string.h>     // strlen(3), memset(3)
#include <stdint.h>     // uint8_t
#include <inttypes.h>   // PRIu8
#include <stdlib.h>     // exit(3)
#include <sys/ioctl.h>  // ioctl(2)

#include "../msr_safe.h"   	// batch data structs
#include "../msr_version.h"	// MSR_SAFE_VERSION_u32

#define MSR_MPERF 0xE7

char const *const allowlist = "0x611 0xFFFFFFFFFFFFFFFF\n";  // PKG_ENERGY_STATUS

static uint8_t const nCPUs = 16;

void set_allowlist()
{
    int fd = open("/dev/cpu/msr_allowlist", O_WRONLY);
    assert(-1 != fd);
    ssize_t nbytes = write(fd, allowlist, strlen(allowlist));
    assert(strlen(allowlist) == nbytes);
    close(fd);
}

void batch( void ){
	struct msr_batch_array b;
	b.version = MSR_SAFE_VERSION_u32;

	struct msr_batch_op o[] = {
		{ .cpu = 2, .op = OP_READ | OP_POLL | OP_INITIAL_MPERF | OP_FINAL_MPERF | OP_POLL_MPERF, .err = 0, .poll_max = 10000, .msr = 0x611, .msrdata = 0, .wmask = 0, .mperf_initial = 0, .mperf_poll = 0, .mperf_final = 0, .msrdata2 = 0 }
	};
	b.numops = sizeof(o) / sizeof( struct msr_batch_op );
	b.ops = o;

	int fd, rc;

	fd = open( "/dev/cpu/msr_batch", O_RDONLY );
	assert(-1 != fd);

	for( size_t i=0; i<100000; i++ ){
		o[0].poll_max = 10000;
		rc = ioctl( fd, X86_IOC_MSR_BATCH, &b );
		assert(-1 != rc);

		printf("%llu %llu %llu %llu %u\n", o[0].mperf_poll, o[0].mperf_final, o[0].msrdata, o[0].msrdata2, o[0].poll_max);
	}
	close( fd );
}

int main()
{
    set_allowlist();
    batch();
    return 0;
}
