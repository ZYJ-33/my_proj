#include"aot1.h"
#include"aot_class.h"
#include"util.h"
#include<cstring>
#include<cstdlib>
#include<climits>
#include<cassert>
#include"./vistors.h"

std::map<u_int64_t, std::shared_ptr<TB>> x86AddrToTb;
TB::TB(FILE* f, AOT_TB* aot_tb, u_int32_t SegBegin):origin_aot_tb(aot_tb)
{
    true_branch = nullptr;
    false_branch = nullptr;
    x86_addr = aot_tb->offset_in_segment + SegBegin;

    auto tb = x86AddrToTb[x86_addr];
    if(tb != nullptr)
    {
        std::cerr<<"x86_addr: "<<std::hex<<x86_addr<<" already in map"<<std::endl;
        error("");
    }

    int save = ftell(f);
    code = new char[aot_tb->tb_cache_size];
    code_size = aot_tb->tb_cache_size;
    fseek(f, aot_tb->tb_cache_offset, SEEK_SET);
    fread(code, code_size, 1, f);
    
    fseek(f, save, SEEK_SET);
}

ListNode<LoongArchInsInfo>* TB::delete_ith_rel(u_int64_t i)
{
    if(rels_valid[i])
    {
        AOT_rel& rel = rels[i];
        u_int64_t ith_insn = rel.tc_offset >> 2;
        u_int64_t count = rel.rel_slots_num;
        ListNode<LoongArchInsInfo>* insn = dis_insns.get(i);
        while(count > 0)
        {
            insn = delete_ith_insn_alongwith_rel(insn, i);
            count -= 1;
        }
        return insn;
    }
    else
        return nullptr;
}

ListNode<LoongArchInsInfo>* TB::delete_ith_insn_alongwith_rel(ListNode<LoongArchInsInfo>* node, u_int64_t i)
{
    u_int64_t offset_of_ith_insn = i << 2;

    u_int64_t j = 0;
    for(auto& rel: rels)
    {
         if (rels_valid[j] && (rel.tc_offset <= offset_of_ith_insn &&  offset_of_ith_insn < rel.tc_offset + (rel.rel_slots_num<<2)))
         {
              rels_valid[j] = false;
              break;
         }
         j += 1;
    }

    j = 0;
    for(auto& rel: rels)
    {
        if(rels_valid[j] && offset_of_ith_insn < rel.tc_offset)
                rel.tc_offset -= 4;
        j += 1;
    }

    for(int i=0; i<2; i++)
    {
        if(origin_aot_tb->jmp_reset_offsets[i] != TB_JMP_RESET_OFFSET_INVALID)
        {
            if(origin_aot_tb->jmp_target_arg[i] > offset_of_ith_insn)
                    origin_aot_tb->jmp_target_arg[i] -= 4;
        }
    }
    return dis_insns.remove(node);
}

ListNode<LoongArchInsInfo>* TB::delete_ith_insn(ListNode<LoongArchInsInfo>* node, u_int64_t i)
{
    u_int64_t offset_of_ith_insn = i << 2;

    u_int64_t j = 0;
    for(auto& rel: rels)
    {
         if (rels_valid[j] && (rel.tc_offset <= offset_of_ith_insn &&  offset_of_ith_insn < rel.tc_offset + (rel.rel_slots_num<<2)))
         {
                std::cerr << " try to delete insn in rel entry: " << i <<"th insn"<< " ignore for now "<<std::endl;
                return node;
         }
         j += 1;
    }

    j = 0;
    for(auto& rel: rels)
    {
        if(rels_valid[j] && (offset_of_ith_insn < rel.tc_offset))
                rel.tc_offset -= 4;
        j += 1;
    }
    for(int i=0; i<2; i++)
    {
        if(origin_aot_tb->jmp_reset_offsets[i] != TB_JMP_RESET_OFFSET_INVALID)
        {
            if(origin_aot_tb->jmp_target_arg[i] > offset_of_ith_insn)
                    origin_aot_tb->jmp_target_arg[i] -= 4;
        }
    }
    return dis_insns.remove(node);
}

void SetNextTB(std::shared_ptr<TB> tb ,std::shared_ptr<TB> branch, bool is_true_branch)
{
    if(is_true_branch)
    {
        assert(tb->true_branch == nullptr);
        tb->true_branch = branch;
    }
    else
    {
        assert(tb->false_branch == nullptr);
        tb->false_branch = branch;
    }
    branch->parents.push_back(tb);
}

Segment::Segment(FILE* f, const AOT_Segment* seg):seg(seg)
{
    int save = ftell(f);

    char lib_name_buf[PATH_MAX];
    fseek(f, seg->lib_name_offset, SEEK_SET);
    fread(lib_name_buf, PATH_MAX, 1, f);
    lib_name = lib_name_buf;
    file_offset = seg->info.file_offset;

    fseek(f, seg->tb_offset, SEEK_SET);
    AOT_TB tmp;
    tbs.reserve(seg->tb_nums);
    _tbs.reserve(seg->tb_nums);
    for(int i=0; i<seg->tb_nums; i++)
    {
        fread(&tmp, sizeof(AOT_TB), 1, f);
        tbs.push_back(tmp);
        auto tb =  std::make_shared<TB>(f, &tbs[i], (seg->info).seg_begin);
        x86AddrToTb[(seg->info).seg_begin + tbs[i].offset_in_segment] = tb;
        _tbs.push_back(tb);
    }
    fseek(f, save, SEEK_SET);
}

void Segment::get_addr(u_int64_t* res, int which, const TB& tb, const std::vector<AOT_rel>& rels)
{
    u_int64_t branch_insn_offset = (tb.origin_aot_tb)->jmp_target_arg[which];
    u_int32_t* branch_insn_ptr = (u_int32_t*)(tb.code+branch_insn_offset);

    for(int i=tb.origin_aot_tb->rel_start_index; i<=tb.origin_aot_tb->rel_end_index; i++)
    {
        const AOT_rel& rel = rels[i];
        if(rel.kind == LOAD_CALL_TARGET
            && rel.tc_offset == branch_insn_offset + 20
            /*&& ((*branch_insn_ptr) & 0x1f) == 21*/)
         {
                *res = rel.extra_addend + (seg->info).seg_begin;
                return;
         }
    }
}

void Segment::TB_Link(std::shared_ptr<TB> tb, bool is_true_branch, const std::vector<AOT_rel>& rels)
{
   u_int64_t x86_addr = -1;
   get_addr(&x86_addr, is_true_branch? 1:0, *tb, rels);
   if (x86_addr == -1)
   {
        if(is_true_branch)
        {
            std::cerr<<"can't find true branch in tb , pc: "<<std::hex<<(seg->info).seg_begin + tb->origin_aot_tb->offset_in_segment;
            exit(1);
        }
        else
        {
            std::cerr<<"can't find false branch in tb , pc: "<<std::hex<<(seg->info).seg_begin + tb->origin_aot_tb->offset_in_segment;
            exit(1);
        }
   }
   std::shared_ptr<TB> next_tb = x86AddrToTb[x86_addr];
   if(next_tb == nullptr)
   {
        std::cerr<<"warning： can't find tb relate to x86_addr: "<<std::hex<<x86_addr<<std::endl;
       // exit(1);
   }
   else
        SetNextTB(tb, next_tb, is_true_branch);
}

void Segment::settle_all_tb(const std::vector<AOT_rel>& rels)
{
    for(auto tb: _tbs)
    {
        const AOT_TB& origin_tb = *tb->origin_aot_tb;
        int true_branch = 1;
        int false_branch = 0;
        u_int64_t seg_begin = (seg->info).seg_begin;

        if(origin_tb.jmp_reset_offsets[true_branch] != TB_JMP_RESET_OFFSET_INVALID 
           && origin_tb.jmp_reset_offsets[false_branch] != TB_JMP_RESET_OFFSET_INVALID)
        {
            TB_Link(tb, true, rels);
            TB_Link(tb, false, rels);
        }
        else if(origin_tb.jmp_reset_offsets[true_branch] != TB_JMP_RESET_OFFSET_INVALID)
            TB_Link(tb, true, rels);
        else if(origin_tb.jmp_reset_offsets[false_branch] != TB_JMP_RESET_OFFSET_INVALID)
            TB_Link(tb, false, rels);
        else
                std::cerr<<"TB with pc: "<<origin_tb.offset_in_segment + seg_begin << "did not have outgoing branch"<<std::endl;
        
    }
}

AOT_File::AOT_File(FILE* f)
{
    DisassmbleVistor v;
    DisassmblePrinterVistor p;
    EntryBlockVistor e;
    testVistor t;
    RemoveTailVistor r;
    TB_AddRels_Vistor add_rels_vistor;

    fseek(f, 0, SEEK_SET);
    fread(&hdr, sizeof(AOT_Header), 1, f);
    fseek(f, hdr.segtab_offset, SEEK_SET);
    AOT_Segment seg_tmp;
    segments.reserve(hdr.segnum);
    _segs.reserve(hdr.segnum);
    for(int i=0; i<hdr.segnum; i++)
    {
        fread(&seg_tmp, sizeof(AOT_Segment), 1, f);
        segments.push_back(seg_tmp);
        _segs.push_back(new Segment(f, &segments[segments.size()-1]));
    }

    fseek(f, hdr.reltable_offset, SEEK_SET);
    AOT_rel rel_tmp;
    rels.reserve(hdr.reltable_num);
    for(int i=0; i<hdr.reltable_num; i++)
    {
        fread(&rel_tmp, sizeof(AOT_rel), 1, f);
        rels.push_back(rel_tmp);
    }

    for(auto seg_ptr: _segs)
    {
        seg_ptr->settle_all_tb(rels);
        add_rels_vistor.start(*seg_ptr, &rels);
        v.start(*seg_ptr);
        r.start(*seg_ptr);
        //e.start(*seg_ptr);
        p.start(*seg_ptr, 0x4017f0, &rels);
        // p.start(*seg_ptr);
    }
}

int main()
{
    FILE* f = fopen("./hello_static.aot", "r+");
    AOT_File a(f);
}
