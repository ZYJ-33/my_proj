#ifndef _AOT_LINK_SEG_H_
#define _AOT_LINK_SEG_H_
#include<cstdlib>
typedef u_int64_t target_ulong;

typedef struct aot_link_info {
    target_ulong pc;
    const void *addr;
    u_int32_t flags;
    /*CPUState*/void *cpu;
} aot_link_info;

#endif
