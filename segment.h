#ifndef __SEGMENT_H_
#define __SEGMENT_H_
#include<cstdlib>
typedef u_int64_t target_ulong;

typedef struct seg_info {
    char *file_name;
    target_ulong file_offset;
    target_ulong seg_begin;
    target_ulong seg_end;
} seg_info;
#endif
