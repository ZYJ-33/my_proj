#ifndef __AOT_H_
#define __AOT_H_

#include "qemu-def.h"
#include "qemu/cutils.h"
#include "segment.h"
#include "latx-version.h"
#include "aot_link_seg.h"
extern char pf_table[];
extern  const char *aot_file_size_optarg;
extern  const char *aot_left_file_minsize_optarg;
/* AOT file layout:
 * +--------------+
 * |  AOT Header  |
 * +--------------+
 * |  AOT Segments|
 * +--------------+
 * |  X86 lib name|
 * +--------------+
 * | AOT TB tables|
 * +--------------+
 * |  Rel tables  |
 * +--------------+
 * |  code caches |
 * +--------------+
 */
#ifdef CONFIG_LATX_DEBUG
#define AOT_VERSION "Version: "LATX_VERSION"-debug"
#else
#define AOT_VERSION "Version: "LATX_VERSION"-release"
#endif
typedef struct aot_header {
    uint32_t segment_table_offset;
    uint32_t segments_num;
    uint32_t rel_table_offset; /* relocation table offset in aot file. */
    uint32_t rel_entry_num; /* relocation entries num. */
} aot_header;

typedef struct aot_segment {
    /* @details.file_offset: segment offset in x86 lib file.
     * @details.seg_bein, @details.segend: its pc ranges in qemu.
     *
     * Note! (lib_flie_name, file_offset) pair is the unique ID of a given
     * segment.
     */
    seg_info details;

    /* x86 library file name(which this segment belongs to) offset in the AOT.
     */
    uint32_t lib_name_offset;

    /* Offset of this segment in AOT. */
    uint32_t segment_tbs_offset;
    uint32_t segment_tbs_num;
    /* Is this segment mapped to memory? */
    bool is_mapped;
} aot_segment;

typedef struct aot_tb {
    /* inner segment offset of this basic block */
    uint32_t offset_in_segment;

    /* Translation Code Cache offset in AOT. */
    void *tb_cache_addr;
    uint32_t tb_cache_offset;
    uint32_t tb_cache_size;

    /* qemu TranslationBlock fields which should be recorded. */
    uint32_t size;
    uint32_t flags;
    uint16_t jmp_reset_offset[2];
    uintptr_t jmp_target_arg[2];
    int32_t rel_start_index;
    int32_t rel_end_index;

    /* segment index of curr tb, its fallthrough tb, and its target tb */
    int segment_idx;
    int fallthru_segment_idx;
    int target_segment_idx;

} aot_tb;

typedef enum aot_rel_kind {

    B_PROLOGUE,
    B_EPILOGUE,
    B_NATIVE_JMP_GLUE2,
    B_NATIVE_ROTATE_FPU_BY,
    B_NATIVE_TO_BT_RET_0,

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

    LOAD_HELPER_END,

    LOAD_TUNNEL_ADDR_BEGIN,
} aot_rel_kind;

typedef struct aot_rel {
    aot_rel_kind kind;

    /* @tc_offset: Offset of this relocation in translated code.
     * @rel_slot_num: How many instruction slots this relocation occupy.
     */
    uint32_t tc_offset;
    uint32_t rel_slots_num;

    /* @x86_rip_offset: Offset of this X86 instruction in x86 baisc block.
     * @extra_addend: extra addend to this relocation.
     *
     * For example, if we want to reloacate *0xdeadbeef(rip)*, we'll fill
     * relocation slots with (rip + 0xdeadbeef). rip can be determined by
     * segment_base_addr + @x86_rip_offset while 0xdeadbeef should be
     * determined by @extra_addend.
     */
    uint32_t x86_rip_offset;
    target_ulong extra_addend;

} aot_rel;

void mk_aot_dir(const char * pathname);
void aot_generate(void);
int aot_load(void);
void aot_tb_register(TranslationBlock *tb);
void aot_do_tb_reloc(TranslationBlock *tb,
    target_ulong seg_begin, target_ulong seg_end);
int aot_get_file_name(char *aot_file, char *buff, int index);
int add_rel_entry(aot_rel_kind kind, uint32_t tc_offset, uint32_t rel_slots_num,
                  uint32_t x86_rip_offset, target_ulong extra_addent);
struct aot_segment *aot_find_segment(char *path, target_ulong seg_begin, target_ulong seg_end, int offset);

#define TB_SET_REL(tb, rel_index)                 \
    do {                                          \
        if ((tb)->rel_start == -1) {                \
            (tb)->rel_start = rel_index;            \
            (tb)->rel_end = rel_index;              \
        } else {                                  \
            assert((tb)->rel_end + 1 == rel_index); \
            (tb)->rel_end = rel_index;              \
        }                                         \
    } while (0)


extern void *aot_buffer;
extern char *aot_file_path;
extern aot_rel *aot_rel_table;
extern void **aot_buffer_all;
extern int aot_buffer_all_num;
#endif
