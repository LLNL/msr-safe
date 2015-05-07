# 1 "__msr_safe.c"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "__msr_safe.c"
# 28 "__msr_safe.c"
# 1 "/usr/include/linux/types.h" 1 3 4



# 1 "/usr/include/asm/types.h" 1 3 4



# 1 "/usr/include/asm-generic/types.h" 1 3 4






# 1 "/usr/include/asm-generic/int-ll64.h" 1 3 4
# 11 "/usr/include/asm-generic/int-ll64.h" 3 4
# 1 "/usr/include/asm/bitsperlong.h" 1 3 4
# 10 "/usr/include/asm/bitsperlong.h" 3 4
# 1 "/usr/include/asm-generic/bitsperlong.h" 1 3 4
# 11 "/usr/include/asm/bitsperlong.h" 2 3 4
# 12 "/usr/include/asm-generic/int-ll64.h" 2 3 4







typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;


__extension__ typedef __signed__ long long __s64;
__extension__ typedef unsigned long long __u64;
# 8 "/usr/include/asm-generic/types.h" 2 3 4



typedef unsigned short umode_t;
# 5 "/usr/include/asm/types.h" 2 3 4
# 5 "/usr/include/linux/types.h" 2 3 4



# 1 "/usr/include/linux/posix_types.h" 1 3 4



# 1 "/usr/include/linux/stddef.h" 1 3 4
# 5 "/usr/include/linux/posix_types.h" 2 3 4
# 36 "/usr/include/linux/posix_types.h" 3 4
typedef struct {
 unsigned long fds_bits [(1024/(8 * sizeof(unsigned long)))];
} __kernel_fd_set;


typedef void (*__kernel_sighandler_t)(int);


typedef int __kernel_key_t;
typedef int __kernel_mqd_t;

# 1 "/usr/include/asm/posix_types.h" 1 3 4



# 1 "/usr/include/asm/posix_types_64.h" 1 3 4
# 10 "/usr/include/asm/posix_types_64.h" 3 4
typedef unsigned long __kernel_ino_t;
typedef unsigned int __kernel_mode_t;
typedef unsigned long __kernel_nlink_t;
typedef long __kernel_off_t;
typedef int __kernel_pid_t;
typedef int __kernel_ipc_pid_t;
typedef unsigned int __kernel_uid_t;
typedef unsigned int __kernel_gid_t;
typedef unsigned long __kernel_size_t;
typedef long __kernel_ssize_t;
typedef long __kernel_ptrdiff_t;
typedef long __kernel_time_t;
typedef long __kernel_suseconds_t;
typedef long __kernel_clock_t;
typedef int __kernel_timer_t;
typedef int __kernel_clockid_t;
typedef int __kernel_daddr_t;
typedef char * __kernel_caddr_t;
typedef unsigned short __kernel_uid16_t;
typedef unsigned short __kernel_gid16_t;


typedef long long __kernel_loff_t;


typedef struct {
 int val[2];
} __kernel_fsid_t;

typedef unsigned short __kernel_old_uid_t;
typedef unsigned short __kernel_old_gid_t;
typedef __kernel_uid_t __kernel_uid32_t;
typedef __kernel_gid_t __kernel_gid32_t;

typedef unsigned long __kernel_old_dev_t;
# 5 "/usr/include/asm/posix_types.h" 2 3 4
# 48 "/usr/include/linux/posix_types.h" 2 3 4
# 9 "/usr/include/linux/types.h" 2 3 4
# 27 "/usr/include/linux/types.h" 3 4
typedef __u16 __le16;
typedef __u16 __be16;
typedef __u32 __le32;
typedef __u32 __be32;
typedef __u64 __le64;
typedef __u64 __be64;

typedef __u16 __sum16;
typedef __u32 __wsum;
# 29 "__msr_safe.c" 2
# 1 "/usr/include/linux/errno.h" 1 3 4



# 1 "/usr/include/asm/errno.h" 1 3 4
# 1 "/usr/include/asm-generic/errno.h" 1 3 4



# 1 "/usr/include/asm-generic/errno-base.h" 1 3 4
# 5 "/usr/include/asm-generic/errno.h" 2 3 4
# 1 "/usr/include/asm/errno.h" 2 3 4
# 5 "/usr/include/linux/errno.h" 2 3 4
# 30 "__msr_safe.c" 2
# 1 "/usr/include/linux/fcntl.h" 1 3 4



# 1 "/usr/include/asm/fcntl.h" 1 3 4
# 1 "/usr/include/asm-generic/fcntl.h" 1 3 4
# 94 "/usr/include/asm-generic/fcntl.h" 3 4
struct f_owner_ex {
 int type;
 pid_t pid;
};
# 139 "/usr/include/asm-generic/fcntl.h" 3 4
struct flock {
 short l_type;
 short l_whence;
 __kernel_off_t l_start;
 __kernel_off_t l_len;
 __kernel_pid_t l_pid;

};
# 156 "/usr/include/asm-generic/fcntl.h" 3 4
struct flock64 {
 short l_type;
 short l_whence;
 __kernel_loff_t l_start;
 __kernel_loff_t l_len;
 __kernel_pid_t l_pid;

};
# 1 "/usr/include/asm/fcntl.h" 2 3 4
# 5 "/usr/include/linux/fcntl.h" 2 3 4
# 31 "__msr_safe.c" 2

# 1 "/usr/include/linux/poll.h" 1 3 4



# 1 "/usr/include/asm/poll.h" 1 3 4
# 1 "/usr/include/asm-generic/poll.h" 1 3 4
# 33 "/usr/include/asm-generic/poll.h" 3 4
struct pollfd {
 int fd;
 short events;
 short revents;
};
# 1 "/usr/include/asm/poll.h" 2 3 4
# 5 "/usr/include/linux/poll.h" 2 3 4
# 33 "__msr_safe.c" 2

# 1 "/usr/include/linux/major.h" 1 3 4
# 35 "__msr_safe.c" 2
# 1 "/usr/include/linux/fs.h" 1 3 4
# 9 "/usr/include/linux/fs.h" 3 4
# 1 "/usr/include/linux/limits.h" 1 3 4
# 10 "/usr/include/linux/fs.h" 2 3 4
# 1 "/usr/include/linux/ioctl.h" 1 3 4



# 1 "/usr/include/asm/ioctl.h" 1 3 4
# 1 "/usr/include/asm-generic/ioctl.h" 1 3 4
# 1 "/usr/include/asm/ioctl.h" 2 3 4
# 5 "/usr/include/linux/ioctl.h" 2 3 4
# 11 "/usr/include/linux/fs.h" 2 3 4
# 1 "/usr/include/linux/blk_types.h" 1 3 4
# 68 "/usr/include/linux/blk_types.h" 3 4
enum bio_rw_flags {
 BIO_RW,
 BIO_RW_FAILFAST_DEV,
 BIO_RW_FAILFAST_TRANSPORT,
 BIO_RW_FAILFAST_DRIVER,

 BIO_RW_AHEAD,
 BIO_RW_BARRIER,
 BIO_RW_SYNCIO,
 BIO_RW_UNPLUG,
 BIO_RW_META,
 BIO_RW_DISCARD,
 BIO_RW_NOIDLE,




 BIO_RW_FLUSH,
 BIO_RW_FUA,




 BIO_RW_THROTTLED,
 BIO_RW_NOMERGE,
};
# 108 "/usr/include/linux/blk_types.h" 3 4
enum rq_flag_bits {
 __REQ_WRITE,
 __REQ_FAILFAST_DEV,
 __REQ_FAILFAST_TRANSPORT,
 __REQ_FAILFAST_DRIVER,

 __REQ_DISCARD,
 __REQ_SORTED,
 __REQ_SOFTBARRIER,
 __REQ_HARDBARRIER,
 __REQ_FUA,
 __REQ_NOMERGE,
 __REQ_STARTED,
 __REQ_DONTPREP,
 __REQ_QUEUED,
 __REQ_ELVPRIV,
 __REQ_FAILED,
 __REQ_QUIET,
 __REQ_PREEMPT,
 __REQ_ORDERED_COLOR,
 __REQ_SYNC,
 __REQ_ALLOCED,
 __REQ_META,
 __REQ_COPY_USER,
 __REQ_INTEGRITY,
 __REQ_NOIDLE,
 __REQ_IO_STAT,
 __REQ_MIXED_MERGE,
 __REQ_FLUSH,
 __REQ_FLUSH_SEQ,
 __REQ_NR_BITS,
};
# 12 "/usr/include/linux/fs.h" 2 3 4
# 37 "/usr/include/linux/fs.h" 3 4
struct fstrim_range {
 __u64 start;
 __u64 len;
 __u64 minlen;
};


struct files_stat_struct {
 unsigned long nr_files;
 unsigned long nr_free_files;
 unsigned long max_files;
};

struct inodes_stat_t {
 int nr_inodes;
 int nr_unused;
 int dummy[5];
};
# 36 "__msr_safe.c" 2
# 46 "__msr_safe.c"
static struct class *msr_class;
static int majordev;

struct smsr_entry{
 loff_t reg;



 u32 write_mask_0;
 u32 write_mask_1;

 int arch;
 const char* strReg;
 const char* strName;
};
# 70 "__msr_safe.c"
# 1 "msr-supplemental.h" 1
# 25 "msr-supplemental.h"
# 1 "./whitelists/table_35_17.h" 1
# 26 "msr-supplemental.h" 2
# 1 "./whitelists/table_35_20.h" 1
# 27 "msr-supplemental.h" 2
# 1 "./whitelists/table_35_23.h" 1
# 28 "msr-supplemental.h" 2
# 1 "./whitelists/table_35_15.h" 1
# 29 "msr-supplemental.h" 2
# 1 "./whitelists/table_35_18.h" 1
# 30 "msr-supplemental.h" 2

# 1 "./whitelists/table_35_24.h" 1
# 32 "msr-supplemental.h" 2
# 1 "./whitelists/table_35_16.h" 1
# 33 "msr-supplemental.h" 2
# 1 "./whitelists/table_35_19.h" 1
# 34 "msr-supplemental.h" 2
# 1 "./whitelists/table_35_22.h" 1
# 35 "msr-supplemental.h" 2
# 71 "__msr_safe.c" 2
# 79 "__msr_safe.c"
typedef enum smsr{


NO_SUCH_SMSR,




WL_TABLE_35_17 p062D_SMSR_TIME_STAMP_COUNTER,p062D_SMSR_PLATFORM_ID,p062D_SMSR_PMC0,p062D_SMSR_PMC1,p062D_SMSR_PMC2,p062D_SMSR_PMC3,p062D_SMSR_PMC4,p062D_SMSR_PMC5,p062D_SMSR_PMC6,p062D_SMSR_PMC7,p062D_SMSR_MPERF,p062D_SMSR_APERF,p062D_SMSR_PERFEVTSEL0,p062D_SMSR_PERFEVTSEL1,p062D_SMSR_PERFEVTSEL2,p062D_SMSR_PERFEVTSEL3,p062D_SMSR_PERFEVTSEL4,p062D_SMSR_PERFEVTSEL5,p062D_SMSR_PERFEVTSEL6,p062D_SMSR_PERFEVTSEL7,p062D_SMSR_PERF_STATUS,p062D_SMSR_PERF_CTL,p062D_SMSR_CLOCK_MODULATION,p062D_SMSR_THERM_INTERRUPT,p062D_SMSR_THERM_STATUS,p062D_SMSR_MISC_ENABLE,p062D_SMSR_TEMPERATURE_TARGET,p062D_SMSR_OFFCORE_RSP_0,p062D_SMSR_OFFCORE_RSP_1,p062D_SMSR_ENERGY_PERF_BIAS,p062D_SMSR_PACKAGE_THERM_STATUS,p062D_SMSR_PACKAGE_THERM_INTERRUPT,p062D_SMSR_FIXED_CTR0,p062D_SMSR_FIXED_CTR1,p062D_SMSR_FIXED_CTR2,p062D_SMSR_PERF_CAPABILITIES,p062D_SMSR_FIXED_CTR_CTRL,p062D_SMSR_PERF_GLOBAL_STATUS,p062D_SMSR_PERF_GLOBAL_CTRL,p062D_SMSR_PERF_GLOBAL_OVF_CTRL,p062D_SMSR_PEBS_ENABLE,p062D_SMSR_PEBS_LD_LAT,p062D_SMSR_RAPL_POWER_UNIT,p062D_SMSR_PKG_POWER_LIMIT,p062D_SMSR_PKG_ENERGY_STATUS,p062D_SMSR_PKG_POWER_INFO,p062D_SMSR_PP0_POWER_LIMIT,p062D_SMSR_PP0_ENERGY_STATUS,




WL_TABLE_35_16 p062A_SMSR_TIME_STAMP_COUNTER,p062A_SMSR_PLATFORM_ID,p062A_SMSR_PMC0,p062A_SMSR_PMC1,p062A_SMSR_PMC2,p062A_SMSR_PMC3,p062A_SMSR_PMC4,p062A_SMSR_PMC5,p062A_SMSR_PMC6,p062A_SMSR_PMC7,p062A_SMSR_MPERF,p062A_SMSR_APERF,p062A_SMSR_PERFEVTSEL0,p062A_SMSR_PERFEVTSEL1,p062A_SMSR_PERFEVTSEL2,p062A_SMSR_PERFEVTSEL3,p062A_SMSR_PERFEVTSEL4,p062A_SMSR_PERFEVTSEL5,p062A_SMSR_PERFEVTSEL6,p062A_SMSR_PERFEVTSEL7,p062A_SMSR_PERF_STATUS,p062A_SMSR_PERF_CTL,p062A_SMSR_CLOCK_MODULATION,p062A_SMSR_THERM_INTERRUPT,p062A_SMSR_THERM_STATUS,p062A_SMSR_MISC_ENABLE,p062A_SMSR_TEMPERATURE_TARGET,p062A_SMSR_OFFCORE_RSP_0,p062A_SMSR_OFFCORE_RSP_1,p062A_SMSR_ENERGY_PERF_BIAS,p062A_SMSR_PACKAGE_THERM_STATUS,p062A_SMSR_PACKAGE_THERM_INTERRUPT,p062A_SMSR_FIXED_CTR0,p062A_SMSR_FIXED_CTR1,p062A_SMSR_FIXED_CTR2,p062A_SMSR_PERF_CAPABILITIES,p062A_SMSR_FIXED_CTR_CTRL,p062A_SMSR_PERF_GLOBAL_STATUS,p062A_SMSR_PERF_GLOBAL_CTRL,p062A_SMSR_PERF_GLOBAL_OVF_CTRL,p062A_SMSR_PEBS_ENABLE,p062A_SMSR_PEBS_LD_LAT,p062A_SMSR_RAPL_POWER_UNIT,p062A_SMSR_PKG_POWER_LIMIT,p062A_SMSR_PKG_ENERGY_STATUS,p062A_SMSR_PKG_POWER_INFO,p062A_SMSR_PP0_POWER_LIMIT,p062A_SMSR_PP0_ENERGY_STATUS,




p063E_SMSR_DRAM_POWER_LIMIT,p063E_SMSR_DRAM_ENERGY_STATUS,p063E_SMSR_DRAM_PERF_STATUS,p063E_SMSR_DRAM_POWER_INFO, p063E_NO_SUCH_SMSR, p063E_SMSR_TIME_STAMP_COUNTER,p063E_SMSR_PLATFORM_ID,p063E_SMSR_PMC0,p063E_SMSR_PMC1,p063E_SMSR_PMC2,p063E_SMSR_PMC3,p063E_SMSR_PMC4,p063E_SMSR_PMC5,p063E_SMSR_PMC6,p063E_SMSR_PMC7,p063E_SMSR_MPERF,p063E_SMSR_APERF,p063E_SMSR_PERFEVTSEL0,p063E_SMSR_PERFEVTSEL1,p063E_SMSR_PERFEVTSEL2,p063E_SMSR_PERFEVTSEL3,p063E_SMSR_PERFEVTSEL4,p063E_SMSR_PERFEVTSEL5,p063E_SMSR_PERFEVTSEL6,p063E_SMSR_PERFEVTSEL7,p063E_SMSR_PERF_STATUS,p063E_SMSR_PERF_CTL,p063E_SMSR_CLOCK_MODULATION,p063E_SMSR_THERM_INTERRUPT,p063E_SMSR_THERM_STATUS,p063E_SMSR_MISC_ENABLE,p063E_SMSR_TEMPERATURE_TARGET,p063E_SMSR_OFFCORE_RSP_0,p063E_SMSR_OFFCORE_RSP_1,p063E_SMSR_ENERGY_PERF_BIAS,p063E_SMSR_PACKAGE_THERM_STATUS,p063E_SMSR_PACKAGE_THERM_INTERRUPT,p063E_SMSR_FIXED_CTR0,p063E_SMSR_FIXED_CTR1,p063E_SMSR_FIXED_CTR2,p063E_SMSR_PERF_CAPABILITIES,p063E_SMSR_FIXED_CTR_CTRL,p063E_SMSR_PERF_GLOBAL_STATUS,p063E_SMSR_PERF_GLOBAL_CTRL,p063E_SMSR_PERF_GLOBAL_OVF_CTRL,p063E_SMSR_PEBS_ENABLE,p063E_SMSR_PEBS_LD_LAT,p063E_SMSR_RAPL_POWER_UNIT,p063E_SMSR_PKG_POWER_LIMIT,p063E_SMSR_PKG_ENERGY_STATUS,p063E_SMSR_PKG_POWER_INFO,p063E_SMSR_PP0_POWER_LIMIT,p063E_SMSR_PP0_ENERGY_STATUS,




p063C_SMSR_TIME_STAMP_COUNTER,p063C_SMSR_PLATFORM_ID,p063C_SMSR_PMC0,p063C_SMSR_PMC1,p063C_SMSR_PMC2,p063C_SMSR_PMC3,p063C_SMSR_PMC4,p063C_SMSR_PMC5,p063C_SMSR_PMC6,p063C_SMSR_PMC7,p063C_SMSR_MPERF,p063C_SMSR_APERF,p063C_SMSR_PERFEVTSEL0,p063C_SMSR_PERFEVTSEL1,p063C_SMSR_PERFEVTSEL2,p063C_SMSR_PERFEVTSEL3,p063C_SMSR_PERFEVTSEL4,p063C_SMSR_PERFEVTSEL5,p063C_SMSR_PERFEVTSEL6,p063C_SMSR_PERFEVTSEL7,p063C_SMSR_PERF_STATUS,p063C_SMSR_PERF_CTL,p063C_SMSR_CLOCK_MODULATION,p063C_SMSR_THERM_INTERRUPT,p063C_SMSR_THERM_STATUS,p063C_SMSR_MISC_ENABLE,p063C_SMSR_TEMPERATURE_TARGET,p063C_SMSR_OFFCORE_RSP_0,p063C_SMSR_OFFCORE_RSP_1,p063C_SMSR_ENERGY_PERF_BIAS,p063C_SMSR_PACKAGE_THERM_STATUS,p063C_SMSR_PACKAGE_THERM_INTERRUPT,p063C_SMSR_FIXED_CTR0,p063C_SMSR_FIXED_CTR1,p063C_SMSR_FIXED_CTR2,p063C_SMSR_PERF_CAPABILITIES,p063C_SMSR_FIXED_CTR_CTRL,p063C_SMSR_PERF_GLOBAL_STATUS,p063C_SMSR_PERF_GLOBAL_CTRL,p063C_SMSR_PERF_GLOBAL_OVF_CTRL,p063C_SMSR_PEBS_ENABLE,p063C_SMSR_PEBS_LD_LAT,p063C_SMSR_RAPL_POWER_UNIT,p063C_SMSR_PKG_POWER_LIMIT,p063C_SMSR_PKG_ENERGY_STATUS,p063C_SMSR_PKG_POWER_INFO,p063C_SMSR_PP0_POWER_LIMIT,p063C_SMSR_PP0_ENERGY_STATUS, WL_TABLE_35_16 p063C_NO_SUCH_SMSR, WL_TABLE_38_23




p0645_SMSR_TIME_STAMP_COUNTER,p0645_SMSR_PLATFORM_ID,p0645_SMSR_PMC0,p0645_SMSR_PMC1,p0645_SMSR_PMC2,p0645_SMSR_PMC3,p0645_SMSR_PMC4,p0645_SMSR_PMC5,p0645_SMSR_PMC6,p0645_SMSR_PMC7,p0645_SMSR_MPERF,p0645_SMSR_APERF,p0645_SMSR_PERFEVTSEL0,p0645_SMSR_PERFEVTSEL1,p0645_SMSR_PERFEVTSEL2,p0645_SMSR_PERFEVTSEL3,p0645_SMSR_PERFEVTSEL4,p0645_SMSR_PERFEVTSEL5,p0645_SMSR_PERFEVTSEL6,p0645_SMSR_PERFEVTSEL7,p0645_SMSR_PERF_STATUS,p0645_SMSR_PERF_CTL,p0645_SMSR_CLOCK_MODULATION,p0645_SMSR_THERM_INTERRUPT,p0645_SMSR_THERM_STATUS,p0645_SMSR_MISC_ENABLE,p0645_SMSR_TEMPERATURE_TARGET,p0645_SMSR_OFFCORE_RSP_0,p0645_SMSR_OFFCORE_RSP_1,p0645_SMSR_ENERGY_PERF_BIAS,p0645_SMSR_PACKAGE_THERM_STATUS,p0645_SMSR_PACKAGE_THERM_INTERRUPT,p0645_SMSR_FIXED_CTR0,p0645_SMSR_FIXED_CTR1,p0645_SMSR_FIXED_CTR2,p0645_SMSR_PERF_CAPABILITIES,p0645_SMSR_FIXED_CTR_CTRL,p0645_SMSR_PERF_GLOBAL_STATUS,p0645_SMSR_PERF_GLOBAL_CTRL,p0645_SMSR_PERF_GLOBAL_OVF_CTRL,p0645_SMSR_PEBS_ENABLE,p0645_SMSR_PEBS_LD_LAT,p0645_SMSR_RAPL_POWER_UNIT,p0645_SMSR_PKG_POWER_LIMIT,p0645_SMSR_PKG_ENERGY_STATUS,p0645_SMSR_PKG_POWER_INFO,p0645_SMSR_PP0_POWER_LIMIT,p0645_SMSR_PP0_ENERGY_STATUS, WL_TABLE_35_16 p0645_NO_SUCH_SMSR, WL_TABLE_38_23




p0646_SMSR_TIME_STAMP_COUNTER,p0646_SMSR_PLATFORM_ID,p0646_SMSR_PMC0,p0646_SMSR_PMC1,p0646_SMSR_PMC2,p0646_SMSR_PMC3,p0646_SMSR_PMC4,p0646_SMSR_PMC5,p0646_SMSR_PMC6,p0646_SMSR_PMC7,p0646_SMSR_MPERF,p0646_SMSR_APERF,p0646_SMSR_PERFEVTSEL0,p0646_SMSR_PERFEVTSEL1,p0646_SMSR_PERFEVTSEL2,p0646_SMSR_PERFEVTSEL3,p0646_SMSR_PERFEVTSEL4,p0646_SMSR_PERFEVTSEL5,p0646_SMSR_PERFEVTSEL6,p0646_SMSR_PERFEVTSEL7,p0646_SMSR_PERF_STATUS,p0646_SMSR_PERF_CTL,p0646_SMSR_CLOCK_MODULATION,p0646_SMSR_THERM_INTERRUPT,p0646_SMSR_THERM_STATUS,p0646_SMSR_MISC_ENABLE,p0646_SMSR_TEMPERATURE_TARGET,p0646_SMSR_OFFCORE_RSP_0,p0646_SMSR_OFFCORE_RSP_1,p0646_SMSR_ENERGY_PERF_BIAS,p0646_SMSR_PACKAGE_THERM_STATUS,p0646_SMSR_PACKAGE_THERM_INTERRUPT,p0646_SMSR_FIXED_CTR0,p0646_SMSR_FIXED_CTR1,p0646_SMSR_FIXED_CTR2,p0646_SMSR_PERF_CAPABILITIES,p0646_SMSR_FIXED_CTR_CTRL,p0646_SMSR_PERF_GLOBAL_STATUS,p0646_SMSR_PERF_GLOBAL_CTRL,p0646_SMSR_PERF_GLOBAL_OVF_CTRL,p0646_SMSR_PEBS_ENABLE,p0646_SMSR_PEBS_LD_LAT,p0646_SMSR_RAPL_POWER_UNIT,p0646_SMSR_PKG_POWER_LIMIT,p0646_SMSR_PKG_ENERGY_STATUS,p0646_SMSR_PKG_POWER_INFO,p0646_SMSR_PP0_POWER_LIMIT,p0646_SMSR_PP0_ENERGY_STATUS, WL_TABLE_35_16 p0646_NO_SUCH_SMSR, p0646_NO_SUCH_SMSR, p0646_NO_SUCH_SMSR,




SMSR_LAST_ENTRY


} smsr_t;



struct smsr_entry whitelist_EMPTY[] = { {0x000,0x0,0x0,0,"0x000","NO_SUCH_SMSR"}, {0x000,0x0,0x0,0,"0x000","SMSR_LAST_ENTRY"} };




struct smsr_entry whitelist_062D[] = { {0x000,0x0,0x0,1,"0x000","NO_SUCH_SMSR"}, WL_TABLE_35_17 {0x010,0x0,0x0,1,"0x010","SMSR_TIME_STAMP_COUNTER"},{0x017,0x0,0x0,1,"0x017","SMSR_PLATFORM_ID"},{0x0C1,0xFFFFFFFF,0xFFFFFFFF,1,"0x0C1","SMSR_PMC0"},{0x0C2,0xFFFFFFFF,0xFFFFFFFF,1,"0x0C2","SMSR_PMC1"},{0x0C3,0xFFFFFFFF,0xFFFFFFFF,1,"0x0C3","SMSR_PMC2"},{0x0C4,0xFFFFFFFF,0xFFFFFFFF,1,"0x0C4","SMSR_PMC3"},{0x0C5,0xFFFFFFFF,0xFFFFFFFF,1,"0x0C5","SMSR_PMC4"},{0x0C6,0xFFFFFFFF,0xFFFFFFFF,1,"0x0C6","SMSR_PMC5"},{0x0C7,0xFFFFFFFF,0xFFFFFFFF,1,"0x0C7","SMSR_PMC6"},{0x0C8,0xFFFFFFFF,0xFFFFFFFF,1,"0x0C8","SMSR_PMC7"},{0x0E7,0x0,0x0,1,"0x0E7","SMSR_MPERF"},{0x0E8,0x0,0x0,1,"0x0E8","SMSR_APERF"},{0x186,0xFFFFFFFF,0x0,1,"0x186","SMSR_PERFEVTSEL0"},{0x187,0xFFFFFFFF,0x0,1,"0x187","SMSR_PERFEVTSEL1"},{0x188,0xFFFFFFFF,0x0,1,"0x188","SMSR_PERFEVTSEL2"},{0x189,0xFFFFFFFF,0x0,1,"0x189","SMSR_PERFEVTSEL3"},{0x18A,0xFFFFFFFF,0x0,1,"0x18A","SMSR_PERFEVTSEL4"},{0x18B,0xFFFFFFFF,0x0,1,"0x18B","SMSR_PERFEVTSEL5"},{0x18C,0xFFFFFFFF,0x0,1,"0x18C","SMSR_PERFEVTSEL6"},{0x18D,0xFFFFFFFF,0x0,1,"0x18D","SMSR_PERFEVTSEL7"},{0x198,0x0,0x0,1,"0x198","SMSR_PERF_STATUS"},{0x199,0x0,0x0,1,"0x199","SMSR_PERF_CTL"},{0x19A,0x00000007,0x0,1,"0x19A","SMSR_CLOCK_MODULATION"},{0x19B,0x01FFFF0F,0x0,1,"0x19B","SMSR_THERM_INTERRUPT"},{0x19C,0x00000AAA,0x0,1,"0x19C","SMSR_THERM_STATUS"},{0x1A0,0x0,0x0,1,"0x1A0","SMSR_MISC_ENABLE"},{0x1A2,0x0,0x0,1,"0x1A2","SMSR_TEMPERATURE_TARGET"},{0x1A6,0xFFFF8FFF,0x0000003F,1,"0x1A6","SMSR_OFFCORE_RSP_0"},{0x1A7,0xFFFF8FFF,0x0000003F,1,"0x1A7","SMSR_OFFCORE_RSP_1"},{0x1B0,0xF,0x0,1,"0x1B0","SMSR_ENERGY_PERF_BIAS"},{0x1B1,0x00000555,0x0,1,"0x1B1","SMSR_PACKAGE_THERM_STATUS"},{0x1B2,0x01FFFF07,0x0,1,"0x1B2","SMSR_PACKAGE_THERM_INTERRUPT"},{0x309,0xFFFFFFFF,0xFFFFFFFF,1,"0x309","SMSR_FIXED_CTR0"},{0x30A,0xFFFFFFFF,0xFFFFFFFF,1,"0x30A","SMSR_FIXED_CTR1"},{0x30B,0xFFFFFFFF,0xFFFFFFFF,1,"0x30B","SMSR_FIXED_CTR2"},{0x345,0x0,0x0,1,"0x345","SMSR_PERF_CAPABILITIES"},{0x38D,0x00000BBB,0x0,1,"0x38D","SMSR_FIXED_CTR_CTRL"},{0x38E,0x0,0x0,1,"0x38E","SMSR_PERF_GLOBAL_STATUS"},{0x38F,0x00000003,0x00000007,1,"0x38F","SMSR_PERF_GLOBAL_CTRL"},{0x390,0x00000003,0xC0000007,1,"0x390","SMSR_PERF_GLOBAL_OVF_CTRL"},{0x3F1,0x0000000F,0x0000000F,1,"0x3F1","SMSR_PEBS_ENABLE"},{0x3F6,0x0000FFFF,0x0,1,"0x3F6","SMSR_PEBS_LD_LAT"},{0x606,0x0,0x0,1,"0x606","SMSR_RAPL_POWER_UNIT"},{0x610,0x00FFFFFF,0x00FFFFFF,1,"0x610","SMSR_PKG_POWER_LIMIT"},{0x611,0x0,0x0,1,"0x611","SMSR_PKG_ENERGY_STATUS"},{0x614,0x0,0x0,1,"0x614","SMSR_PKG_POWER_INFO"},{0x638,0x00FFFFFF,0x0,1,"0x638","SMSR_PP0_POWER_LIMIT"},{0x639,0x0,0x0,1,"0x639","SMSR_PP0_ENERGY_STATUS"}, {0x000,0x0,0x0,1,"0x000","SMSR_LAST_ENTRY"} };




struct smsr_entry whitelist_062A[] = { {0x000,0x0,0x0,2,"0x000","NO_SUCH_SMSR"}, WL_TABLE_35_16 {0x010,0x0,0x0,2,"0x010","SMSR_TIME_STAMP_COUNTER"},{0x017,0x0,0x0,2,"0x017","SMSR_PLATFORM_ID"},{0x0C1,0xFFFFFFFF,0xFFFFFFFF,2,"0x0C1","SMSR_PMC0"},{0x0C2,0xFFFFFFFF,0xFFFFFFFF,2,"0x0C2","SMSR_PMC1"},{0x0C3,0xFFFFFFFF,0xFFFFFFFF,2,"0x0C3","SMSR_PMC2"},{0x0C4,0xFFFFFFFF,0xFFFFFFFF,2,"0x0C4","SMSR_PMC3"},{0x0C5,0xFFFFFFFF,0xFFFFFFFF,2,"0x0C5","SMSR_PMC4"},{0x0C6,0xFFFFFFFF,0xFFFFFFFF,2,"0x0C6","SMSR_PMC5"},{0x0C7,0xFFFFFFFF,0xFFFFFFFF,2,"0x0C7","SMSR_PMC6"},{0x0C8,0xFFFFFFFF,0xFFFFFFFF,2,"0x0C8","SMSR_PMC7"},{0x0E7,0x0,0x0,2,"0x0E7","SMSR_MPERF"},{0x0E8,0x0,0x0,2,"0x0E8","SMSR_APERF"},{0x186,0xFFFFFFFF,0x0,2,"0x186","SMSR_PERFEVTSEL0"},{0x187,0xFFFFFFFF,0x0,2,"0x187","SMSR_PERFEVTSEL1"},{0x188,0xFFFFFFFF,0x0,2,"0x188","SMSR_PERFEVTSEL2"},{0x189,0xFFFFFFFF,0x0,2,"0x189","SMSR_PERFEVTSEL3"},{0x18A,0xFFFFFFFF,0x0,2,"0x18A","SMSR_PERFEVTSEL4"},{0x18B,0xFFFFFFFF,0x0,2,"0x18B","SMSR_PERFEVTSEL5"},{0x18C,0xFFFFFFFF,0x0,2,"0x18C","SMSR_PERFEVTSEL6"},{0x18D,0xFFFFFFFF,0x0,2,"0x18D","SMSR_PERFEVTSEL7"},{0x198,0x0,0x0,2,"0x198","SMSR_PERF_STATUS"},{0x199,0x0,0x0,2,"0x199","SMSR_PERF_CTL"},{0x19A,0x00000007,0x0,2,"0x19A","SMSR_CLOCK_MODULATION"},{0x19B,0x01FFFF0F,0x0,2,"0x19B","SMSR_THERM_INTERRUPT"},{0x19C,0x00000AAA,0x0,2,"0x19C","SMSR_THERM_STATUS"},{0x1A0,0x0,0x0,2,"0x1A0","SMSR_MISC_ENABLE"},{0x1A2,0x0,0x0,2,"0x1A2","SMSR_TEMPERATURE_TARGET"},{0x1A6,0xFFFF8FFF,0x0000003F,2,"0x1A6","SMSR_OFFCORE_RSP_0"},{0x1A7,0xFFFF8FFF,0x0000003F,2,"0x1A7","SMSR_OFFCORE_RSP_1"},{0x1B0,0xF,0x0,2,"0x1B0","SMSR_ENERGY_PERF_BIAS"},{0x1B1,0x00000555,0x0,2,"0x1B1","SMSR_PACKAGE_THERM_STATUS"},{0x1B2,0x01FFFF07,0x0,2,"0x1B2","SMSR_PACKAGE_THERM_INTERRUPT"},{0x309,0xFFFFFFFF,0xFFFFFFFF,2,"0x309","SMSR_FIXED_CTR0"},{0x30A,0xFFFFFFFF,0xFFFFFFFF,2,"0x30A","SMSR_FIXED_CTR1"},{0x30B,0xFFFFFFFF,0xFFFFFFFF,2,"0x30B","SMSR_FIXED_CTR2"},{0x345,0x0,0x0,2,"0x345","SMSR_PERF_CAPABILITIES"},{0x38D,0x00000BBB,0x0,2,"0x38D","SMSR_FIXED_CTR_CTRL"},{0x38E,0x0,0x0,2,"0x38E","SMSR_PERF_GLOBAL_STATUS"},{0x38F,0x00000003,0x00000007,2,"0x38F","SMSR_PERF_GLOBAL_CTRL"},{0x390,0x00000003,0xC0000007,2,"0x390","SMSR_PERF_GLOBAL_OVF_CTRL"},{0x3F1,0x0000000F,0x0000000F,2,"0x3F1","SMSR_PEBS_ENABLE"},{0x3F6,0x0000FFFF,0x0,2,"0x3F6","SMSR_PEBS_LD_LAT"},{0x606,0x0,0x0,2,"0x606","SMSR_RAPL_POWER_UNIT"},{0x610,0x00FFFFFF,0x00FFFFFF,2,"0x610","SMSR_PKG_POWER_LIMIT"},{0x611,0x0,0x0,2,"0x611","SMSR_PKG_ENERGY_STATUS"},{0x614,0x0,0x0,2,"0x614","SMSR_PKG_POWER_INFO"},{0x638,0x00FFFFFF,0x0,2,"0x638","SMSR_PP0_POWER_LIMIT"},{0x639,0x0,0x0,2,"0x639","SMSR_PP0_ENERGY_STATUS"}, {0x000,0x0,0x0,2,"0x000","SMSR_LAST_ENTRY"} };




struct smsr_entry whitelist_063E[] = { {0x000,0x0,0x0,3,"0x000","NO_SUCH_SMSR"}, {0x618,0x00FFFFFF,0x0,3,"0x618","SMSR_DRAM_POWER_LIMIT"},{0x619,0x0,0x0,3,"0x619","SMSR_DRAM_ENERGY_STATUS"},{0x61B,0x0,0x0,3,"0x61B","SMSR_DRAM_PERF_STATUS"},{0x61C,0x0,0x0,3,"0x61C","SMSR_DRAM_POWER_INFO"}, {0x000,0x0,0x0,3,"0x000","NO_SUCH_SMSR"}, {0x010,0x0,0x0,3,"0x010","SMSR_TIME_STAMP_COUNTER"},{0x017,0x0,0x0,3,"0x017","SMSR_PLATFORM_ID"},{0x0C1,0xFFFFFFFF,0xFFFFFFFF,3,"0x0C1","SMSR_PMC0"},{0x0C2,0xFFFFFFFF,0xFFFFFFFF,3,"0x0C2","SMSR_PMC1"},{0x0C3,0xFFFFFFFF,0xFFFFFFFF,3,"0x0C3","SMSR_PMC2"},{0x0C4,0xFFFFFFFF,0xFFFFFFFF,3,"0x0C4","SMSR_PMC3"},{0x0C5,0xFFFFFFFF,0xFFFFFFFF,3,"0x0C5","SMSR_PMC4"},{0x0C6,0xFFFFFFFF,0xFFFFFFFF,3,"0x0C6","SMSR_PMC5"},{0x0C7,0xFFFFFFFF,0xFFFFFFFF,3,"0x0C7","SMSR_PMC6"},{0x0C8,0xFFFFFFFF,0xFFFFFFFF,3,"0x0C8","SMSR_PMC7"},{0x0E7,0x0,0x0,3,"0x0E7","SMSR_MPERF"},{0x0E8,0x0,0x0,3,"0x0E8","SMSR_APERF"},{0x186,0xFFFFFFFF,0x0,3,"0x186","SMSR_PERFEVTSEL0"},{0x187,0xFFFFFFFF,0x0,3,"0x187","SMSR_PERFEVTSEL1"},{0x188,0xFFFFFFFF,0x0,3,"0x188","SMSR_PERFEVTSEL2"},{0x189,0xFFFFFFFF,0x0,3,"0x189","SMSR_PERFEVTSEL3"},{0x18A,0xFFFFFFFF,0x0,3,"0x18A","SMSR_PERFEVTSEL4"},{0x18B,0xFFFFFFFF,0x0,3,"0x18B","SMSR_PERFEVTSEL5"},{0x18C,0xFFFFFFFF,0x0,3,"0x18C","SMSR_PERFEVTSEL6"},{0x18D,0xFFFFFFFF,0x0,3,"0x18D","SMSR_PERFEVTSEL7"},{0x198,0x0,0x0,3,"0x198","SMSR_PERF_STATUS"},{0x199,0x0,0x0,3,"0x199","SMSR_PERF_CTL"},{0x19A,0x00000007,0x0,3,"0x19A","SMSR_CLOCK_MODULATION"},{0x19B,0x01FFFF0F,0x0,3,"0x19B","SMSR_THERM_INTERRUPT"},{0x19C,0x00000AAA,0x0,3,"0x19C","SMSR_THERM_STATUS"},{0x1A0,0x0,0x0,3,"0x1A0","SMSR_MISC_ENABLE"},{0x1A2,0x0,0x0,3,"0x1A2","SMSR_TEMPERATURE_TARGET"},{0x1A6,0xFFFF8FFF,0x0000003F,3,"0x1A6","SMSR_OFFCORE_RSP_0"},{0x1A7,0xFFFF8FFF,0x0000003F,3,"0x1A7","SMSR_OFFCORE_RSP_1"},{0x1B0,0xF,0x0,3,"0x1B0","SMSR_ENERGY_PERF_BIAS"},{0x1B1,0x00000555,0x0,3,"0x1B1","SMSR_PACKAGE_THERM_STATUS"},{0x1B2,0x01FFFF07,0x0,3,"0x1B2","SMSR_PACKAGE_THERM_INTERRUPT"},{0x309,0xFFFFFFFF,0xFFFFFFFF,3,"0x309","SMSR_FIXED_CTR0"},{0x30A,0xFFFFFFFF,0xFFFFFFFF,3,"0x30A","SMSR_FIXED_CTR1"},{0x30B,0xFFFFFFFF,0xFFFFFFFF,3,"0x30B","SMSR_FIXED_CTR2"},{0x345,0x0,0x0,3,"0x345","SMSR_PERF_CAPABILITIES"},{0x38D,0x00000BBB,0x0,3,"0x38D","SMSR_FIXED_CTR_CTRL"},{0x38E,0x0,0x0,3,"0x38E","SMSR_PERF_GLOBAL_STATUS"},{0x38F,0x00000003,0x00000007,3,"0x38F","SMSR_PERF_GLOBAL_CTRL"},{0x390,0x00000003,0xC0000007,3,"0x390","SMSR_PERF_GLOBAL_OVF_CTRL"},{0x3F1,0x0000000F,0x0000000F,3,"0x3F1","SMSR_PEBS_ENABLE"},{0x3F6,0x0000FFFF,0x0,3,"0x3F6","SMSR_PEBS_LD_LAT"},{0x606,0x0,0x0,3,"0x606","SMSR_RAPL_POWER_UNIT"},{0x610,0x00FFFFFF,0x00FFFFFF,3,"0x610","SMSR_PKG_POWER_LIMIT"},{0x611,0x0,0x0,3,"0x611","SMSR_PKG_ENERGY_STATUS"},{0x614,0x0,0x0,3,"0x614","SMSR_PKG_POWER_INFO"},{0x638,0x00FFFFFF,0x0,3,"0x638","SMSR_PP0_POWER_LIMIT"},{0x639,0x0,0x0,3,"0x639","SMSR_PP0_ENERGY_STATUS"}, {0x000,0x0,0x0,3,"0x000","SMSR_LAST_ENTRY"} };




struct smsr_entry whitelist_063C[] = { {0x000,0x0,0x0,4,"0x000","NO_SUCH_SMSR"}, {0x010,0x0,0x0,4,"0x010","SMSR_TIME_STAMP_COUNTER"},{0x017,0x0,0x0,4,"0x017","SMSR_PLATFORM_ID"},{0x0C1,0xFFFFFFFF,0xFFFFFFFF,4,"0x0C1","SMSR_PMC0"},{0x0C2,0xFFFFFFFF,0xFFFFFFFF,4,"0x0C2","SMSR_PMC1"},{0x0C3,0xFFFFFFFF,0xFFFFFFFF,4,"0x0C3","SMSR_PMC2"},{0x0C4,0xFFFFFFFF,0xFFFFFFFF,4,"0x0C4","SMSR_PMC3"},{0x0C5,0xFFFFFFFF,0xFFFFFFFF,4,"0x0C5","SMSR_PMC4"},{0x0C6,0xFFFFFFFF,0xFFFFFFFF,4,"0x0C6","SMSR_PMC5"},{0x0C7,0xFFFFFFFF,0xFFFFFFFF,4,"0x0C7","SMSR_PMC6"},{0x0C8,0xFFFFFFFF,0xFFFFFFFF,4,"0x0C8","SMSR_PMC7"},{0x0E7,0x0,0x0,4,"0x0E7","SMSR_MPERF"},{0x0E8,0x0,0x0,4,"0x0E8","SMSR_APERF"},{0x186,0xFFFFFFFF,0x0,4,"0x186","SMSR_PERFEVTSEL0"},{0x187,0xFFFFFFFF,0x0,4,"0x187","SMSR_PERFEVTSEL1"},{0x188,0xFFFFFFFF,0x0,4,"0x188","SMSR_PERFEVTSEL2"},{0x189,0xFFFFFFFF,0x0,4,"0x189","SMSR_PERFEVTSEL3"},{0x18A,0xFFFFFFFF,0x0,4,"0x18A","SMSR_PERFEVTSEL4"},{0x18B,0xFFFFFFFF,0x0,4,"0x18B","SMSR_PERFEVTSEL5"},{0x18C,0xFFFFFFFF,0x0,4,"0x18C","SMSR_PERFEVTSEL6"},{0x18D,0xFFFFFFFF,0x0,4,"0x18D","SMSR_PERFEVTSEL7"},{0x198,0x0,0x0,4,"0x198","SMSR_PERF_STATUS"},{0x199,0x0,0x0,4,"0x199","SMSR_PERF_CTL"},{0x19A,0x00000007,0x0,4,"0x19A","SMSR_CLOCK_MODULATION"},{0x19B,0x01FFFF0F,0x0,4,"0x19B","SMSR_THERM_INTERRUPT"},{0x19C,0x00000AAA,0x0,4,"0x19C","SMSR_THERM_STATUS"},{0x1A0,0x0,0x0,4,"0x1A0","SMSR_MISC_ENABLE"},{0x1A2,0x0,0x0,4,"0x1A2","SMSR_TEMPERATURE_TARGET"},{0x1A6,0xFFFF8FFF,0x0000003F,4,"0x1A6","SMSR_OFFCORE_RSP_0"},{0x1A7,0xFFFF8FFF,0x0000003F,4,"0x1A7","SMSR_OFFCORE_RSP_1"},{0x1B0,0xF,0x0,4,"0x1B0","SMSR_ENERGY_PERF_BIAS"},{0x1B1,0x00000555,0x0,4,"0x1B1","SMSR_PACKAGE_THERM_STATUS"},{0x1B2,0x01FFFF07,0x0,4,"0x1B2","SMSR_PACKAGE_THERM_INTERRUPT"},{0x309,0xFFFFFFFF,0xFFFFFFFF,4,"0x309","SMSR_FIXED_CTR0"},{0x30A,0xFFFFFFFF,0xFFFFFFFF,4,"0x30A","SMSR_FIXED_CTR1"},{0x30B,0xFFFFFFFF,0xFFFFFFFF,4,"0x30B","SMSR_FIXED_CTR2"},{0x345,0x0,0x0,4,"0x345","SMSR_PERF_CAPABILITIES"},{0x38D,0x00000BBB,0x0,4,"0x38D","SMSR_FIXED_CTR_CTRL"},{0x38E,0x0,0x0,4,"0x38E","SMSR_PERF_GLOBAL_STATUS"},{0x38F,0x00000003,0x00000007,4,"0x38F","SMSR_PERF_GLOBAL_CTRL"},{0x390,0x00000003,0xC0000007,4,"0x390","SMSR_PERF_GLOBAL_OVF_CTRL"},{0x3F1,0x0000000F,0x0000000F,4,"0x3F1","SMSR_PEBS_ENABLE"},{0x3F6,0x0000FFFF,0x0,4,"0x3F6","SMSR_PEBS_LD_LAT"},{0x606,0x0,0x0,4,"0x606","SMSR_RAPL_POWER_UNIT"},{0x610,0x00FFFFFF,0x00FFFFFF,4,"0x610","SMSR_PKG_POWER_LIMIT"},{0x611,0x0,0x0,4,"0x611","SMSR_PKG_ENERGY_STATUS"},{0x614,0x0,0x0,4,"0x614","SMSR_PKG_POWER_INFO"},{0x638,0x00FFFFFF,0x0,4,"0x638","SMSR_PP0_POWER_LIMIT"},{0x639,0x0,0x0,4,"0x639","SMSR_PP0_ENERGY_STATUS"}, WL_TABLE_35_16 {0x000,0x0,0x0,4,"0x000","NO_SUCH_SMSR"}, WL_TABLE_38_23 {0x000,0x0,0x0,4,"0x000","SMSR_LAST_ENTRY"} };




struct smsr_entry whitelist_0645[] = { {0x000,0x0,0x0,5,"0x000","NO_SUCH_SMSR"}, {0x010,0x0,0x0,5,"0x010","SMSR_TIME_STAMP_COUNTER"},{0x017,0x0,0x0,5,"0x017","SMSR_PLATFORM_ID"},{0x0C1,0xFFFFFFFF,0xFFFFFFFF,5,"0x0C1","SMSR_PMC0"},{0x0C2,0xFFFFFFFF,0xFFFFFFFF,5,"0x0C2","SMSR_PMC1"},{0x0C3,0xFFFFFFFF,0xFFFFFFFF,5,"0x0C3","SMSR_PMC2"},{0x0C4,0xFFFFFFFF,0xFFFFFFFF,5,"0x0C4","SMSR_PMC3"},{0x0C5,0xFFFFFFFF,0xFFFFFFFF,5,"0x0C5","SMSR_PMC4"},{0x0C6,0xFFFFFFFF,0xFFFFFFFF,5,"0x0C6","SMSR_PMC5"},{0x0C7,0xFFFFFFFF,0xFFFFFFFF,5,"0x0C7","SMSR_PMC6"},{0x0C8,0xFFFFFFFF,0xFFFFFFFF,5,"0x0C8","SMSR_PMC7"},{0x0E7,0x0,0x0,5,"0x0E7","SMSR_MPERF"},{0x0E8,0x0,0x0,5,"0x0E8","SMSR_APERF"},{0x186,0xFFFFFFFF,0x0,5,"0x186","SMSR_PERFEVTSEL0"},{0x187,0xFFFFFFFF,0x0,5,"0x187","SMSR_PERFEVTSEL1"},{0x188,0xFFFFFFFF,0x0,5,"0x188","SMSR_PERFEVTSEL2"},{0x189,0xFFFFFFFF,0x0,5,"0x189","SMSR_PERFEVTSEL3"},{0x18A,0xFFFFFFFF,0x0,5,"0x18A","SMSR_PERFEVTSEL4"},{0x18B,0xFFFFFFFF,0x0,5,"0x18B","SMSR_PERFEVTSEL5"},{0x18C,0xFFFFFFFF,0x0,5,"0x18C","SMSR_PERFEVTSEL6"},{0x18D,0xFFFFFFFF,0x0,5,"0x18D","SMSR_PERFEVTSEL7"},{0x198,0x0,0x0,5,"0x198","SMSR_PERF_STATUS"},{0x199,0x0,0x0,5,"0x199","SMSR_PERF_CTL"},{0x19A,0x00000007,0x0,5,"0x19A","SMSR_CLOCK_MODULATION"},{0x19B,0x01FFFF0F,0x0,5,"0x19B","SMSR_THERM_INTERRUPT"},{0x19C,0x00000AAA,0x0,5,"0x19C","SMSR_THERM_STATUS"},{0x1A0,0x0,0x0,5,"0x1A0","SMSR_MISC_ENABLE"},{0x1A2,0x0,0x0,5,"0x1A2","SMSR_TEMPERATURE_TARGET"},{0x1A6,0xFFFF8FFF,0x0000003F,5,"0x1A6","SMSR_OFFCORE_RSP_0"},{0x1A7,0xFFFF8FFF,0x0000003F,5,"0x1A7","SMSR_OFFCORE_RSP_1"},{0x1B0,0xF,0x0,5,"0x1B0","SMSR_ENERGY_PERF_BIAS"},{0x1B1,0x00000555,0x0,5,"0x1B1","SMSR_PACKAGE_THERM_STATUS"},{0x1B2,0x01FFFF07,0x0,5,"0x1B2","SMSR_PACKAGE_THERM_INTERRUPT"},{0x309,0xFFFFFFFF,0xFFFFFFFF,5,"0x309","SMSR_FIXED_CTR0"},{0x30A,0xFFFFFFFF,0xFFFFFFFF,5,"0x30A","SMSR_FIXED_CTR1"},{0x30B,0xFFFFFFFF,0xFFFFFFFF,5,"0x30B","SMSR_FIXED_CTR2"},{0x345,0x0,0x0,5,"0x345","SMSR_PERF_CAPABILITIES"},{0x38D,0x00000BBB,0x0,5,"0x38D","SMSR_FIXED_CTR_CTRL"},{0x38E,0x0,0x0,5,"0x38E","SMSR_PERF_GLOBAL_STATUS"},{0x38F,0x00000003,0x00000007,5,"0x38F","SMSR_PERF_GLOBAL_CTRL"},{0x390,0x00000003,0xC0000007,5,"0x390","SMSR_PERF_GLOBAL_OVF_CTRL"},{0x3F1,0x0000000F,0x0000000F,5,"0x3F1","SMSR_PEBS_ENABLE"},{0x3F6,0x0000FFFF,0x0,5,"0x3F6","SMSR_PEBS_LD_LAT"},{0x606,0x0,0x0,5,"0x606","SMSR_RAPL_POWER_UNIT"},{0x610,0x00FFFFFF,0x00FFFFFF,5,"0x610","SMSR_PKG_POWER_LIMIT"},{0x611,0x0,0x0,5,"0x611","SMSR_PKG_ENERGY_STATUS"},{0x614,0x0,0x0,5,"0x614","SMSR_PKG_POWER_INFO"},{0x638,0x00FFFFFF,0x0,5,"0x638","SMSR_PP0_POWER_LIMIT"},{0x639,0x0,0x0,5,"0x639","SMSR_PP0_ENERGY_STATUS"}, WL_TABLE_35_16 {0x000,0x0,0x0,5,"0x000","NO_SUCH_SMSR"}, WL_TABLE_38_23 {0x000,0x0,0x0,5,"0x000","SMSR_LAST_ENTRY"} };




struct smsr_entry whitelist_0646[] = { {0x000,0x0,0x0,6,"0x000","NO_SUCH_SMSR"}, {0x010,0x0,0x0,6,"0x010","SMSR_TIME_STAMP_COUNTER"},{0x017,0x0,0x0,6,"0x017","SMSR_PLATFORM_ID"},{0x0C1,0xFFFFFFFF,0xFFFFFFFF,6,"0x0C1","SMSR_PMC0"},{0x0C2,0xFFFFFFFF,0xFFFFFFFF,6,"0x0C2","SMSR_PMC1"},{0x0C3,0xFFFFFFFF,0xFFFFFFFF,6,"0x0C3","SMSR_PMC2"},{0x0C4,0xFFFFFFFF,0xFFFFFFFF,6,"0x0C4","SMSR_PMC3"},{0x0C5,0xFFFFFFFF,0xFFFFFFFF,6,"0x0C5","SMSR_PMC4"},{0x0C6,0xFFFFFFFF,0xFFFFFFFF,6,"0x0C6","SMSR_PMC5"},{0x0C7,0xFFFFFFFF,0xFFFFFFFF,6,"0x0C7","SMSR_PMC6"},{0x0C8,0xFFFFFFFF,0xFFFFFFFF,6,"0x0C8","SMSR_PMC7"},{0x0E7,0x0,0x0,6,"0x0E7","SMSR_MPERF"},{0x0E8,0x0,0x0,6,"0x0E8","SMSR_APERF"},{0x186,0xFFFFFFFF,0x0,6,"0x186","SMSR_PERFEVTSEL0"},{0x187,0xFFFFFFFF,0x0,6,"0x187","SMSR_PERFEVTSEL1"},{0x188,0xFFFFFFFF,0x0,6,"0x188","SMSR_PERFEVTSEL2"},{0x189,0xFFFFFFFF,0x0,6,"0x189","SMSR_PERFEVTSEL3"},{0x18A,0xFFFFFFFF,0x0,6,"0x18A","SMSR_PERFEVTSEL4"},{0x18B,0xFFFFFFFF,0x0,6,"0x18B","SMSR_PERFEVTSEL5"},{0x18C,0xFFFFFFFF,0x0,6,"0x18C","SMSR_PERFEVTSEL6"},{0x18D,0xFFFFFFFF,0x0,6,"0x18D","SMSR_PERFEVTSEL7"},{0x198,0x0,0x0,6,"0x198","SMSR_PERF_STATUS"},{0x199,0x0,0x0,6,"0x199","SMSR_PERF_CTL"},{0x19A,0x00000007,0x0,6,"0x19A","SMSR_CLOCK_MODULATION"},{0x19B,0x01FFFF0F,0x0,6,"0x19B","SMSR_THERM_INTERRUPT"},{0x19C,0x00000AAA,0x0,6,"0x19C","SMSR_THERM_STATUS"},{0x1A0,0x0,0x0,6,"0x1A0","SMSR_MISC_ENABLE"},{0x1A2,0x0,0x0,6,"0x1A2","SMSR_TEMPERATURE_TARGET"},{0x1A6,0xFFFF8FFF,0x0000003F,6,"0x1A6","SMSR_OFFCORE_RSP_0"},{0x1A7,0xFFFF8FFF,0x0000003F,6,"0x1A7","SMSR_OFFCORE_RSP_1"},{0x1B0,0xF,0x0,6,"0x1B0","SMSR_ENERGY_PERF_BIAS"},{0x1B1,0x00000555,0x0,6,"0x1B1","SMSR_PACKAGE_THERM_STATUS"},{0x1B2,0x01FFFF07,0x0,6,"0x1B2","SMSR_PACKAGE_THERM_INTERRUPT"},{0x309,0xFFFFFFFF,0xFFFFFFFF,6,"0x309","SMSR_FIXED_CTR0"},{0x30A,0xFFFFFFFF,0xFFFFFFFF,6,"0x30A","SMSR_FIXED_CTR1"},{0x30B,0xFFFFFFFF,0xFFFFFFFF,6,"0x30B","SMSR_FIXED_CTR2"},{0x345,0x0,0x0,6,"0x345","SMSR_PERF_CAPABILITIES"},{0x38D,0x00000BBB,0x0,6,"0x38D","SMSR_FIXED_CTR_CTRL"},{0x38E,0x0,0x0,6,"0x38E","SMSR_PERF_GLOBAL_STATUS"},{0x38F,0x00000003,0x00000007,6,"0x38F","SMSR_PERF_GLOBAL_CTRL"},{0x390,0x00000003,0xC0000007,6,"0x390","SMSR_PERF_GLOBAL_OVF_CTRL"},{0x3F1,0x0000000F,0x0000000F,6,"0x3F1","SMSR_PEBS_ENABLE"},{0x3F6,0x0000FFFF,0x0,6,"0x3F6","SMSR_PEBS_LD_LAT"},{0x606,0x0,0x0,6,"0x606","SMSR_RAPL_POWER_UNIT"},{0x610,0x00FFFFFF,0x00FFFFFF,6,"0x610","SMSR_PKG_POWER_LIMIT"},{0x611,0x0,0x0,6,"0x611","SMSR_PKG_ENERGY_STATUS"},{0x614,0x0,0x0,6,"0x614","SMSR_PKG_POWER_INFO"},{0x638,0x00FFFFFF,0x0,6,"0x638","SMSR_PP0_POWER_LIMIT"},{0x639,0x0,0x0,6,"0x639","SMSR_PP0_ENERGY_STATUS"}, WL_TABLE_35_16 {0x000,0x0,0x0,6,"0x000","NO_SUCH_SMSR"}, {0x000,0x0,0x0,6,"0x000","NO_SUCH_SMSR"}, {0x000,0x0,0x0,6,"0x000","NO_SUCH_SMSR"}, {0x000,0x0,0x0,6,"0x000","SMSR_LAST_ENTRY"} };


static int init=0;
static int arch=0;
struct smsr_entry *whitelist=((void *)0);




static char *Version = "1.3.0";

static ssize_t show_version(struct class *cls, char *buf)
{
 sprintf(buf, "%s\n", Version);
 return strlen(buf) + 1;
}

static ssize_t show_avail(struct class *cls, char *buf)
{
 smsr_t entry;
 for (entry = 0; entry < SMSR_LAST_ENTRY; entry++){
  if ( (whitelist[entry].reg != 0x000) && (whitelist[entry].arch == arch) ){
   strlcat(buf, whitelist[entry].strReg, PAGE_SIZE);
   strlcat(buf, " ", PAGE_SIZE);
   strlcat(buf, whitelist[entry].strName, PAGE_SIZE);
   strlcat(buf, "\n", PAGE_SIZE);
  }

 }

 return strlen(buf) + 1;
}

static struct class_attribute class_attr[] = {
 __ATTR(version, 0644, show_version, ((void *)0)),
 __ATTR(avail, 0644, show_avail, ((void *)0)),
 __ATTR_NULL
};


static struct class smsr_class =
{
 .name = "smsr",
 .owner = THIS_MODULE,
 .class_attrs = class_attr
};



static void
get_cpuid(uint32_t leaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx)
{
        asm volatile(
                "xchg %%rbx, %%rdi\n\tcpuid\n\txchg %%rbx, %%rdi"
                        :"=a" (*eax), "=D" (*ebx), "=c" (*ecx), "=d" (*edx)
                        :"a" (leaf)
                );
}

static int
getArch (void)
{
 uint32_t regs[4];
 uint32_t cpu_type, cpu_model, cpu_family, cpu_stepping;
 uint32_t cpu_family_extended, cpu_model_extended;
 uint32_t cpu_family_adjusted, cpu_model_adjusted;
 char string[80];
 const char *arches[8];
        int arrayLength=7;
 int i;

 get_cpuid( 1, &regs[0], &regs[1], &regs[2], &regs[3] );

 cpu_family = (((uint32_t)(regs[0])&((((uint32_t)1<<(((11))-((8))+1))-1)<<((8))))>>(8));
        cpu_family_extended = (((uint32_t)(regs[0])&((((uint32_t)1<<(((27))-((20))+1))-1)<<((20))))>>(20));
 cpu_model = (((uint32_t)(regs[0])&((((uint32_t)1<<(((7))-((4))+1))-1)<<((4))))>>(4));
 cpu_model_extended = (((uint32_t)(regs[0])&((((uint32_t)1<<(((19))-((16))+1))-1)<<((16))))>>(16));
 cpu_type = (((uint32_t)(regs[0])&((((uint32_t)1<<(((13))-((12))+1))-1)<<((12))))>>(12));
 cpu_stepping = (((uint32_t)(regs[0])&((((uint32_t)1<<(((3))-((0))+1))-1)<<((0))))>>(0));

 if(cpu_family == 0xF){
  cpu_family_adjusted = cpu_family + cpu_family_extended;
 }else{
  cpu_family_adjusted = cpu_family;
 }

 if(cpu_family == 0x6 || cpu_family == 0xF){
  cpu_model_adjusted = (cpu_model_extended << 4) + cpu_model;
 }else{
  cpu_model_adjusted = cpu_model;
 }

 sprintf(string, "%02x_%X", cpu_family_adjusted, cpu_model_adjusted);

 arches[0]= "";
 arches[1]= "06_2D";
        arches[2]= "06_2A";
        arches[3]= "06_3E";
        arches[4]= "06_3C";
        arches[5]= "06_45";
        arches[6]= "06_46";


        for(i=1; i< arrayLength; i++)
        {
         if(strncmp(string,arches[i], strlen(arches[i])) == 0)
          return i;
        }

 return 0;
}

u16 get_whitelist_entry(loff_t reg)
{
 smsr_t entry;

 for (entry = 0; entry < SMSR_LAST_ENTRY; entry++){
  if ( whitelist[entry].reg == reg && whitelist[entry].arch == arch ){
   return entry;
  }
 }
 return 0;
}

static long msr_ioctl(struct file *file, unsigned int ioc, unsigned long arg)
{

 (void)file;
 (void)ioc;
 (void)arg;
 return -25;
}

static loff_t msr_seek(struct file *file, loff_t offset, int orig)
{
        loff_t ret;
        struct inode *inode = file->f_mapping->host;

        mutex_lock(&inode->i_mutex);
        switch (orig) {
        case 0:
                file->f_pos = offset;
                ret = file->f_pos;
                break;
        case 1:
                file->f_pos += offset;
                ret = file->f_pos;
                break;
        default:
                ret = -22;
        }
        mutex_unlock(&inode->i_mutex);
        return ret;
}


static ssize_t msr_read(struct file *file, char __user *buf,
   size_t count, loff_t *ppos)
{
 u32 __user *tmp = (u32 __user *) buf;
 u32 data[2];
 smsr_t idx;
 loff_t reg = *ppos;
 int cpu = iminor(file->f_path.dentry->d_inode);
 int err = -13;



 if (count != 8){
  return -22;
 }

 idx = get_whitelist_entry( reg );

 if(idx){
  err = rdmsr_safe_on_cpu(cpu, reg, &data[0], &data[1]);
  if (!err){
   if (copy_to_user(tmp, &data, 8)) {
    err = -14;
   }
   else {
    err = 0;
   }
  }
 }
 return err ? err : 8;
}

static ssize_t msr_write(struct file *file, const char __user *buf,
    size_t count, loff_t *ppos)
{
 const u32 __user *tmp = (const u32 __user *)buf;
 u32 data[2];
 smsr_t idx;
 loff_t reg = *ppos;
 int cpu = iminor(file->f_path.dentry->d_inode);
 int err = -13;



 if (count != 8)
  return -22;

 idx = get_whitelist_entry( reg );

 if(idx){


  if((whitelist[idx].write_mask_0 | whitelist[idx].write_mask_1 )) {
   if (copy_from_user(&data, tmp, 8)) {
    err = -14;
   } else {
    data[0] &= whitelist[idx].write_mask_0;
    data[1] &= whitelist[idx].write_mask_1;
    err = wrmsr_safe_on_cpu(cpu, reg, data[0], data[1]);
   }
  } else {
   err = -30;
  }
 }
 return err ? err : 8;
}


static int msr_open(struct inode *inode, struct file *file)
{
 unsigned int cpu;
 struct cpuinfo_x86 *c;
 int ret = 0;

 cpu = iminor(file->f_path.dentry->d_inode);

 if (cpu >= nr_cpu_ids || !cpu_online(cpu)) {
  ret = -6;
  goto out;
 }
 c = &cpu_data(cpu);
 if (!cpu_has(c, X86_FEATURE_MSR))
  ret = -5;
out:
 return ret;
}




static const struct file_operations msr_fops = {
 .owner = THIS_MODULE,
 .read = msr_read,
 .write = msr_write,
 .open = msr_open,
 .llseek = msr_seek,
 .unlocked_ioctl = msr_ioctl,
 .compat_ioctl = msr_ioctl,
};

static int __cpuinit msr_device_create(int cpu)
{
 struct device *dev;

 dev = device_create(msr_class, ((void *)0), MKDEV(majordev, cpu), ((void *)0),
       "msr_safe%d", cpu);
 return IS_ERR(dev) ? PTR_ERR(dev) : 0;
}

static void msr_device_destroy(int cpu)
{
 device_destroy(msr_class, MKDEV(majordev, cpu));
}

static int __cpuinit msr_class_cpu_callback(struct notifier_block *nfb,
    unsigned long action, void *hcpu)
{
 unsigned int cpu = (unsigned long)hcpu;
 int err = 0;

 switch (action) {
 case CPU_UP_PREPARE:
  err = msr_device_create(cpu);
  break;
 case CPU_UP_CANCELED:
 case CPU_UP_CANCELED_FROZEN:
 case CPU_DEAD:
  msr_device_destroy(cpu);
  break;
 }
 return notifier_from_errno(err);
}

static struct notifier_block __refdata msr_class_cpu_notifier = {
 .notifier_call = msr_class_cpu_callback,
};

static char *msr_devnode(struct device *dev, mode_t *mode)
{
 return kasprintf(GFP_KERNEL, "cpu/%u/msr_safe", MINOR(dev->devt));
}

static int __init msr_init(void)
{
 int i, err = 0;
 int status;
 i = 0;

 majordev = __register_chrdev(0, 0, NR_CPUS, "cpu/msr_safe", &msr_fops);
 if (majordev < 0) {
  printk(KERN_ERR "msr_safe: unable to register device number\n");
  err = -16;
  goto out;
 }
 msr_class = class_create(THIS_MODULE, "msr_safe");
 if (IS_ERR(msr_class)) {
  err = PTR_ERR(msr_class);
  goto out_chrdev;
 }
 msr_class->devnode = msr_devnode;
 for_each_online_cpu(i) {
  err = msr_device_create(i);
  if (err != 0)
   goto out_class;
 }
 register_hotcpu_notifier(&msr_class_cpu_notifier);

 err = 0;

 if(!init)
 {
  init=1;
  arch=getArch();
  switch(arch) {
  case 1:
   whitelist=whitelist_062D;
   break;
  case 2:
   whitelist=whitelist_062A;
   break;
  case 3:
   whitelist=whitelist_063E;
   break;
  case 4:
   whitelist=whitelist_063C;
   break;
  case 5:
   whitelist=whitelist_0645;
   break;
  case 6:
   whitelist=whitelist_0646;
   break;
  default:
   whitelist=whitelist_EMPTY;
   break;
  }
 }


 status = class_register(&smsr_class);
 if (status < 0)
  printk("Registering smsr class failed\n");

 goto out;

out_class:
 i = 0;
 for_each_online_cpu(i)
  msr_device_destroy(i);
 class_destroy(msr_class);
out_chrdev:
 __unregister_chrdev(majordev, 0, NR_CPUS, "cpu/msr_safe");
out:
 return err;
}

static void __exit msr_exit(void)
{
 int cpu = 0;

 class_unregister(&smsr_class);


 for_each_online_cpu(cpu)
  msr_device_destroy(cpu);
 class_destroy(msr_class);
 __unregister_chrdev(majordev, 0, NR_CPUS, "cpu/msr_safe");
 unregister_hotcpu_notifier(&msr_class_cpu_notifier);
}

module_init(msr_init);
module_exit(msr_exit)

MODULE_AUTHOR("Kathleen Shoga <shoga1@llnl.gov>");
MODULE_DESCRIPTION("x86 sanitized MSR driver");
MODULE_LICENSE("GPL");
