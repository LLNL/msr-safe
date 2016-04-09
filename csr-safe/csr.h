/* csr.h
 *
 * Copyright (c) 2015-2016, Lawrence Livermore National Security, LLC. LLNL-CODE-645430
 * Produced at Lawrence Livermore National Laboratory  
 * Written by  Barry Rountree, rountree@llnl.gov
 *             Scott Walker,   walker91@llnl.gov
 *             Marty McFadden, mcfadden8@llnl.gov
 *			   Kathleen Shoga, shoga1@llnl.gov
 *
 * All rights reserved. 
 * 
 * This file is part of msr-safe.
 * 
 * msr-safe is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 * 
 * msr-safe is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License along
 * with msr-safe.  If not, see <http://www.gnu.org/licenses/>. 
 *
 * This material is based upon work supported by the U.S. Department
 * of Energy's Lawrence Livermore National Laboratory. Office of
 * Science, under Award number DE-AC52-07NA27344.
 *
 */

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
