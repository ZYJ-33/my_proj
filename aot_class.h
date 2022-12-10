#ifndef __AOT_CLASS
#define __AOT_CLASS
#include "aot1.h"
#include <cstdlib>
#include <memory>
#include <map>
#include "./disassmbler.h"
#include "./linklist.h"

class TB;
class TB_Vistor;

class TB
{
    public:
        char* code;
        u_int32_t code_size;
        std::shared_ptr<TB> true_branch;
        std::shared_ptr<TB> false_branch;
        std::vector<std::shared_ptr<TB>> parents;
        std::vector<AOT_rel> rels;
        LinkList<LoongArchInsInfo> dis_insns;
        ListNode<LoongArchInsInfo>* delete_ith_insn(ListNode<LoongArchInsInfo>* node, u_int64_t i);
        //std::vector<LoongArchInsInfo> dis_insns;
        AOT_TB* origin_aot_tb;
        u_int64_t x86_addr;
        TB(FILE* f, AOT_TB* aot_tb, u_int32_t SegBegin);
        void visit(TB_Vistor& vistor);
};

class Segment
{
    public:
        /*lib_name and file_offset are primary key to identify a segment in origin x86 elf file */
        std::string lib_name;
        target_ulong file_offset;
        std::vector<AOT_TB> tbs;
        std::vector<std::shared_ptr<TB>> _tbs;
        const AOT_Segment* seg;
        Segment(FILE* f, const AOT_Segment* seg);
        void get_addr(u_int64_t* res, int which, const TB& tb, const std::vector<AOT_rel>& rels);
        void settle_all_tb(const std::vector<AOT_rel>& rels);
        void TB_Link(std::shared_ptr<TB> tb, bool is_true_branch, const std::vector<AOT_rel>& rels);
};

class AOT_File
{
    public:
            AOT_Header hdr;
            std::vector<AOT_rel> rels;
            std::vector<AOT_Segment> segments;
            std::vector<Segment*> _segs;
            AOT_File(FILE* aot_file);
};
#endif
