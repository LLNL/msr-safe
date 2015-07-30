#ifndef MSR_BATCH_INC
#define MSR_BATCH_INC 1
#include <asm/msr.h>

extern void msrbatch_cleanup(void);
extern int msrbatch_init(void);
#endif /* MSR_BATCH_INC */
