#ifndef MSR_WHITELIST_INC
#define MSR_WHITELIST_INC 1
/* ----------------------------------------------------------------------- *
 * Copyright 2000-2008 H. Peter Anvin - All Rights Reserved
 * Copyright 2009 Intel Corporation; author: H. Peter Anvin
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, Inc., 675 Mass Ave, Cambridge MA 02139,
 * USA; either version 2 of the License, or (at your option) any later
 * version; incorporated herein by reference.
 * ----------------------------------------------------------------------- */

/*
 * Interface definitions
 */
int msr_whitelist_init(void);
int msr_whitelist_cleanup(void);
u64 msr_whitelist_readmask(loff_t reg);
u64 msr_whitelist_writemask(loff_t reg);

#endif /* MSR_WHITELIST_INC */
