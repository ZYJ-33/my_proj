#ifndef __AOT_H
#define __AOT_H
#include <cstdlib>
#include <string>
#include <vector>
#define TB_JMP_RESET_OFFSET_INVALID 0xffff
#define uintptr_t u_int64_t

typedef u_int64_t target_ulong;
typedef struct AOT_Header
{
    u_int32_t segtab_offset;
    u_int32_t segnum;
    u_int32_t reltable_offset;
    u_int32_t reltable_num;
} AOT_Header;

typedef struct AOT_TB
{
    u_int32_t offset_in_segment;
    
    void* tb_cache_addr; //userless?
    u_int32_t tb_cache_offset;
    u_int32_t tb_cache_size;

    u_int32_t size;
    u_int32_t flags;
    u_int16_t jmp_reset_offsets[2];
    uintptr_t jmp_target_arg[2];
    int32_t rel_start_index;
    int32_t rel_end_index;
    
    int segment_idx;
    int fallthru_segment_idx;
    int target_segment_idx;
} AOT_TB;


typedef enum aot_rel_kind {

    B_PROLOGUE,
    B_EPILOGUE,
    B_NATIVE_JMP_GLUE2,

    LOAD_TB_ADDR,
    LOAD_RIP_OFF,
    LOAD_CALL_TARGET,
    LOAD_SYSCALL_OPTIMIZE_CONFIRM,

    /* Helper functions relocation. Assume we need to call a helper in our tb,
     * we'll load its address into a tmp reg and then jmp to that address, but
     * if latx has changed its code and all helper address has changed, all aot
     * tb will need to be relocated. */
    LOAD_HELPER_BEGIN,

    LOAD_HELPER_TRACE_SESSION_BEGIN,
    LOAD_HELPER_UPDATE_MXCSR_STATUS,
    LOAD_HELPER_FPATAN,
    LOAD_HELPER_FPTAN,
    LOAD_HELPER_FPREM,
    LOAD_HELPER_FPREM1,
    LOAD_HELPER_FRNDINT,
    LOAD_HELPER_F2XM1,
    LOAD_HELPER_FXTRACT,
    LOAD_HELPER_FYL2X,
    LOAD_HELPER_FYL2XP1,
    LOAD_HELPER_FSINCOS,
    LOAD_HELPER_FSIN,
    LOAD_HELPER_FCOS,
    LOAD_HELPER_FBLD_ST0,
    LOAD_HELPER_FBST_ST0,
    LOAD_HELPER_FXSAVE,
    LOAD_HELPER_FXRSTOR,
    LOAD_HELPER_CONVERT_FPREGS_X80_TO_64,
    LOAD_HELPER_CONVERT_FPREGS_64_TO_X80,
    LOAD_HELPER_UPDATE_FP_STATUS,
    LOAD_HELPER_CPUID,
    LOAD_HELPER_RAISE_INT,
    LOAD_HELPER_RAISE_SYSCALL,

    LOAD_HOST_POW,
    LOAD_HOST_SIN,
    LOAD_HOST_COS,
    LOAD_HOST_ATAN2,
    LOAD_HOST_LOGB,
    LOAD_HOST_LOG2,
    LOAD_HOST_SINCOS,
    LOAD_HOST_PFTABLE,
    LOAD_HOST_LATLOCK,
    LOAD_HOST_RAISE_EX,

    LOAD_HELPER_END,

    LOAD_TUNNEL_ADDR_BEGIN,
} aot_rel_kind;


typedef struct AOT_rel
{
    public:
     aot_rel_kind kind;
     u_int32_t tc_offset;
     u_int32_t rel_slots_num;

     u_int32_t x86_rip_offset;
     target_ulong extra_addend;
} AOT_rel;

static const char* rel_kind2name[] = {
    [B_PROLOGUE] = "B_PROLOGUE",
    [B_EPILOGUE] = "B_EPILOGUE",
    [B_NATIVE_JMP_GLUE2] = "B_NATIVE_JMP_GLUE2",

    [LOAD_TB_ADDR] = "LOAD_TB_ADDR",
    [LOAD_RIP_OFF] = "LOAD_RIP_OFF",
    [LOAD_CALL_TARGET] = "LOAD_CALL_TARGET",
    [LOAD_SYSCALL_OPTIMIZE_CONFIRM] = "LOAD_SYSCALL_OPTIMIZE_CONFIRM",
    
    
    [LOAD_HELPER_TRACE_SESSION_BEGIN] = "LOAD_HELPER_TRACE_SESSION_BEGIN",
    [LOAD_HELPER_UPDATE_MXCSR_STATUS] = "LOAD_HELPER_UPDATE_MXCSR_STATUS",
    [LOAD_HELPER_FPATAN] = "LOAD_HELPER_FPATAN",
    [LOAD_HELPER_FPTAN] = "LOAD_HELPER_FPTAN",
    [LOAD_HELPER_FPREM] = "LOAD_HELPER_FPREM",
    [LOAD_HELPER_FPREM1] = "LOAD_HELPER_FPREM1",
    [LOAD_HELPER_FRNDINT] = "LOAD_HELPER_FRNDINT",
    [LOAD_HELPER_F2XM1] = "LOAD_HELPER_F2XM1",
    [LOAD_HELPER_FXTRACT] = "LOAD_HELPER_FXTRACT",
    [LOAD_HELPER_FYL2X] = "LOAD_HELPER_FYL2X",
    [LOAD_HELPER_FYL2XP1] = "LOAD_HELPER_FYL2XP1",
    [LOAD_HELPER_FSINCOS] = "LOAD_HELPER_FSINCOS",
    [LOAD_HELPER_FSIN] = "LOAD_HELPER_FSIN",
    [LOAD_HELPER_FCOS] = "LOAD_HELPER_FCOS",
    [LOAD_HELPER_FBLD_ST0] = "LOAD_HELPER_FBLD_ST0",
    [LOAD_HELPER_FBST_ST0] = "LOAD_HELPER_FBST_ST0",
    [LOAD_HELPER_FXSAVE] = "LOAD_HELPER_FXSAVE",
    [LOAD_HELPER_FXRSTOR] = "LOAD_HELPER_FXRSTOR",
    [LOAD_HELPER_CONVERT_FPREGS_X80_TO_64] = "LOAD_HELPER_CONVERT_FPREGS_X80_TO_64",
    [LOAD_HELPER_CONVERT_FPREGS_64_TO_X80] = "LOAD_HELPER_CONVERT_FPREGS_64_TO_X80",
    [LOAD_HELPER_UPDATE_FP_STATUS] = "LOAD_HELPER_UPDATE_FP_STATUS",
    [LOAD_HELPER_CPUID] = "LOAD_HELPER_CPUID",
    [LOAD_HELPER_RAISE_INT] = "LOAD_HELPER_RAISE_INT",
    [LOAD_HELPER_RAISE_SYSCALL] = "LOAD_HELPER_RAISE_SYSCALL",

    [LOAD_HOST_POW] = "LOAD_HOST_POW",
    [LOAD_HOST_SIN] = "LOAD_HOST_SIN",
    [LOAD_HOST_COS] = "LOAD_HOST_COS",
    [LOAD_HOST_ATAN2] = "LOAD_HOST_ATAN2",
    [LOAD_HOST_LOGB] = "LOAD_HOST_LOGB",
    [LOAD_HOST_LOG2] = "LOAD_HOST_LOG2",
    [LOAD_HOST_SINCOS] = "LOAD_HOST_SINCOS",
    [LOAD_HOST_PFTABLE] = "LOAD_HOST_PFTABLE",
    [LOAD_HOST_LATLOCK] = "LOAD_HOST_LATLOCK",
    [LOAD_HOST_RAISE_EX] = "LOAD_HOST_RAISE_EX",

    [LOAD_HELPER_END] = "LOAD_HELPER_END",

    [LOAD_TUNNEL_ADDR_BEGIN] = "LOAD_TUNNEL_ADDR_BEGIN",
    
    

};

typedef struct AOT_SegInfo
{
    public:
    char* file;
    target_ulong file_offset;
    target_ulong seg_begin;
    target_ulong seg_end;
} AOT_SegInfo;

typedef struct AOT_Segment
{
    public:
    AOT_SegInfo info;
    u_int32_t lib_name_offset;
    u_int32_t tb_offset;
    u_int32_t tb_nums;
    bool ismap;
} AOT_Segment;
#endif
