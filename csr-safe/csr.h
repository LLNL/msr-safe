// Author: Scott Walker
// csr.h
// headers for csr safe access driver

#ifndef CSR_HFILE_INC
#define CSR_HFILE_INC 1
#include <linux/types.h>
#include <linux/ioctl.h>
#include <asm/io.h>

#define __NODENAME "CSR_SAFE\n"
#define __VERSTRING "alpha 0.5"
#define CSRSAFE_DIR "cpu/csr_safe"
#define PCIENER 555

struct csrsafe_bdf {
	__u8 bus;
	__u8 device;
	__u8 function;
	__u8 socket;
};

struct csr_batch_op {
	__u8  bus;
	__u8  device;
	__u8  function;
	__u8  socket;
	__u16 offset;
	__s32 err;
	__u8  isread;
        __u64 csrdata;
	__u64 wmask;
	__u8 size;
};

struct csr_batch_array {
	__u32 numops;
	struct csr_batch_op *ops;
};

#define CSRSAFE_8086_BATCH _IOWR('a', 0x05, struct csr_batch_array)

__u8 csr_core_bus(const __u32 socket);
__u8 csr_uncore_bus(const __u32 socket);
void __iomem *__pcie_ioremap(__u8 bus, __u8 device, __u8 func);

#endif /* CSR_HFILE_INC */
