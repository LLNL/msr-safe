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

#ifndef CSR_WHITELIST_H_INCLUDE
#define CSR_WHITELIST_H_INCLUDE

#include <linux/hashtable.h>
#include <linux/types.h>

#include "csr_safe.h"

struct whitelist_entry
{
    /// @brief Writemask for the CSR.
    __u64 wmask;
    // bits 11:0 CSR (offset)
    // bits 15:12 function
    // bits 20:16 device
    // bits 23:21 reserved, must be 0
    // bits 32:24 bus
    // bits 63:32 inherits wmask from this csr
    __u64 csr;
    /// @brief The data of the CSR.
    __u64 *csrdata;
    /// @brief The iomap for this register on socket 0.
    void __iomem *maps0;
    /// @brief The iomap for this register on socket 1.
    void __iomem *maps1;
    struct hlist_node hlist;
};

int csr_whitelist_init(void);

void csr_whitelist_cleanup(void);

struct whitelist_entry *csr_whitelist_entry(struct csr_batch_op *op);

void __iomem *csr_whitelist_map(__u8 socket,
                                __u8 bus,
                                __u8 device,
                                __u8 function,
                                loff_t reg);

#endif
