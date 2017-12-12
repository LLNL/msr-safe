/*
 *
 * Copyright (c) 2015-2017, Lawrence Livermore National Security, LLC.
 * LLNL-CODE-645430
 *
 * Produced at Lawrence Livermore National Laboratory
 * Written by  Barry Rountree, rountree@llnl.gov
 *             Scott Walker,   walker91@llnl.gov
 *             Marty McFadden, mcfadden8@llnl.gov
 *             Kathleen Shoga, shoga1@llnl.gov
 *
 * All rights reserved.
 *
 * This file is part of msr-safe.
 *
 * msr-safe is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * msr-safe is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with msr-safe. If not, see <http://www.gnu.org/licenses/>.
 *
 * This material is based upon work supported by the U.S. Department of
 * Energy's Lawrence Livermore National Laboratory. Office of Science, under
 * Award number DE-AC52-07NA27344.
 *
 */

#ifndef CSR_H_INCLUDE
#define CSR_H_INCLUDE

#include <asm/io.h>
#include <linux/ioctl.h>
#include <linux/types.h>

#define __NODENAME "CSR_SAFE\n"
#define __VERSTRING "alpha 0.5"
#define CSRSAFE_DIR "cpu/csr_safe"
#define PCIENER 555
#define CSRSAFE_8086_BATCH _IOWR('a', 0x05, struct csr_batch_array)

/// @brief Structure holding information about socket, bus, device, and function.
struct csrsafe_bdf
{
    /// @brief Bus number.
    __u8 bus;
    /// @brief Device number.
    __u8 device;
    /// @brief Function number.
    __u8 function;
    /// @brief Socket identifier.
    __u8 socket;
};

/// @brief Structure holding information for a single read/write operation
/// to various CSRs.
struct csr_batch_op
{
    /// @brief Bus number.
    __u8 bus;
    /// @brief Device number.
    __u8 device;
    /// @brief Function number.
    __u8 function;
    /// @brief Offset of CSR to perform operation.
    __u16 offset;
    /// @brief Socket where read/write will be executed.
    __u8 socket;
    /// @brief Stores error code.
    __s32 err;
    /// @brief Identify if operation is read or write.
    __u8 isread;
    /// @brief Stores input to or result from operation.
    __u64 csrdata;
    /// @brief Writemask applied to write.
    __u64 wmask;
    /// @brief Size
    __u8 size;
};

/// @brief Structure holding multiple read/write operations to various CSRs.
struct csr_batch_array
{
    /// @brief Number of operations to execute.
    __u32 numops;
    /// @brief Array of length numops of operations to execute.
    struct csr_batch_op *ops;
};

/// @brief csr_core_bus
///
/// @param [in] socket Socket identifier.
///
/// @return Type u8
__u8 csr_core_bus(const __u32 socket);

/// @brief csr_uncore_bus
///
/// @param [in] socket Socket identifier.
///
/// @return Type u8
__u8 csr_uncore_bus(const __u32 socket);

/// @brief pcie_ioremap
///
/// @param [in] bus Bus number.
///
/// @param [in] device Device number.
///
/// @param [in] func Function number.
///
/// @return iomem
void __iomem *__pcie_ioremap(__u8 bus,
                             __u8 device,
                             __u8 func);

#endif
