#ifndef MSR_WHITELIST_INC
#define MSR_WHITELIST_INC 1
/*
 * MSR Whitelist Interface definitions
 */
int msr_whitelist_init(void);
int msr_whitelist_cleanup(void);
u64 msr_whitelist_readmask(loff_t reg);
u64 msr_whitelist_writemask(loff_t reg);

#endif /* MSR_WHITELIST_INC */
