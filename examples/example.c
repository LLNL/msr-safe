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
#include <unistd.h>     // write(2)
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
		{ .cpu = 2,
            .op             = OP_READ | OP_POLL | OP_INITIAL_MPERF | OP_FINAL_MPERF | OP_POLL_MPERF | OP_INITIAL_APERF | OP_FINAL_APERF | OP_POLL_APERF,
            .err            = 0,
            .poll_max       = 10000,
            .msr            = 0x611,
            .msrdata        = 0,
            .wmask          = 0,
            .mperf_initial  = 0,
            .mperf_poll     = 0,
            .mperf_final    = 0,
            .aperf_initial  = 0,
            .aperf_poll     = 0,
            .aperf_final    = 0,
            .msrdata2       = 0 }
	};
	b.numops = sizeof(o) / sizeof( struct msr_batch_op );
	b.ops = o;

	int fd, rc;

	fd = open( "/dev/cpu/msr_batch", O_RDONLY );
	assert(-1 != fd);

    printf("cpu op err poll_max msr msrdata wmask mi mp mf ai ap af msrdata2\n");
	for( size_t i=0; i<100000; i++ ){
		o[0].poll_max = 10000;
		rc = ioctl( fd, X86_IOC_MSR_BATCH, &b );
		assert(-1 != rc);

        //      cpu  op err p  m   md    w   mi   mp   mf   ai   ap   af md2
        printf("%hu %hu %d %u %u %llu %llu %llu %llu %llu %llu %llu %llu %llu\n",
                o[0].cpu, o[0].op, o[0].err, o[0].poll_max, o[0].msr, o[0].msrdata, o[0].wmask,
                o[0].mperf_initial, o[0].mperf_poll, o[0].mperf_final,
                o[0].aperf_initial, o[0].aperf_poll, o[0].aperf_final,
                o[0].msrdata2);

	}
	close( fd );
}

int main()
{
    set_allowlist();
    batch();
    return 0;
}
/*
struct msr_batch_op
{
    __u16 cpu;              // In: CPU to execute {rd/wr}msr instruction
    __u16 op;               // In: OR at least one of the following:
                            //   OP_WRITE, OP_READ, OP_POLL, OP_INITIAL_MPERF,
                            //   OP_FINAL_MPERF, OP_POLL_MPERF
                            //   OP_INITIAL_APERF, OP_FINAL_APERF, OP_POLL_APERF
    __s32 err;              // Out: set if error occurred with this operation
    __u32 poll_max;         // In/Out:  Max/remaining poll attempts
    __u32 msr;              // In: MSR Address to perform operation
    __u64 msrdata;          // In/Out: Input/Result to/from operation
    __u64 wmask;            // Out: Write mask applied to wrmsr
    __u64 mperf_initial;    // Out: reference clock reading at the start of the op
    __u64 mperf_poll;       // Out: reference clock reading at start of final poll
    __u64 mperf_final;      // Out: reference clock reading at the end of r/w/p
    __u64 aperf_initial;    // Out: cycle count reading at the start of the op
    __u64 aperf_poll;       // Out: cycle count reading at start of final poll
    __u64 aperf_final;      // Out: cycle count reading at the end of r/w/p
    __u64 msrdata2;         // Out: last polled reading
};

typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;

#ifdef __GNUC__
__extension__ typedef __signed__ long long __s64;
__extension__ typedef unsigned long long __u64;
#else
typedef __signed__ long long __s64;
typedef unsigned long long __u64;
#endif
*/

