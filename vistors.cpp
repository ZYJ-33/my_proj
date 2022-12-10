#include "./vistors.h"
#include<cstdlib>
#include<cstring>

extern std::map<u_int64_t, std::shared_ptr<TB>> x86AddrToTb;
void TB::visit(TB_Vistor& vistor)
{
    vistor.visit(*this);
}

void TB_AddRels_Vistor::visit(TB& tb)
{
    u_int64_t start_index = tb.origin_aot_tb->rel_start_index; 
    u_int64_t end_index = tb.origin_aot_tb->rel_end_index;
    tb.rels.reserve(end_index - start_index + 1);
    while(start_index <= end_index)
        tb.rels.push_back((*rels)[start_index++]);
}

void TB_AddRels_Vistor::start(Segment& seg, const std::vector<AOT_rel>* rels)
{
    this->rels = rels;
    for(auto tb_ptr : seg._tbs)
        visit(*tb_ptr);
}

void testVistor::visit(TB& tb)
{
     u_int64_t i = 0;
     for(auto iter = tb.dis_insns.begin(); iter != tb.dis_insns.end();)
     {
        LoongArchInsInfo* insn = iter->data;
        if(insn->opc == OPC_ANDI 
           && insn->dst_reg == 0
           && insn->srcs_regs[0] == 0 
           && insn->ui == 0)
        {
             iter = tb.delete_ith_insn(iter, i);
        }
        else
        {
                iter = iter->next;
                i += 1;
        }
     }
}

void testVistor::start(Segment& seg)
{
    for(auto tb_ptr : seg._tbs)
        visit(*tb_ptr);
}

void EntryBlockVistor::start(Segment& seg)
{
    for(auto tb_ptr : seg._tbs)
        visit(*tb_ptr);
}

void EntryBlockVistor::visit(TB& tb)
{
    if(tb.parents.size() == 0)
            entrys.push_back(tb.x86_addr);
}

void DisassmblePrinterVistor::start(Segment& seg, const std::vector<AOT_rel>* rels)
{
    this->rels = rels;
    this->seg_start = seg.seg->info.seg_begin;
    for(auto tb_ptr: seg._tbs)
         (*tb_ptr).visit(*this);
}

void DisassmblePrinterVistor::start(Segment& seg, u_int64_t entry, const std::vector<AOT_rel>* rels)
{
    this->rels = rels;
    std::vector<std::shared_ptr<TB>> queue;
    std::shared_ptr<TB> cur = x86AddrToTb[entry];
    queue.push_back(cur);
    
    int cur_head = 0;
    std::set<u_int64_t> seen;
    seen.insert(cur->x86_addr);

    while(cur_head < queue.size())
    {
        int count = queue.size()-cur_head;
        while(count > 0)
        {
            std::shared_ptr<TB> head = queue[cur_head];
            head->visit(*this);
            if(head->true_branch != nullptr && seen.find(head->true_branch->x86_addr) == seen.end())
            {
                    queue.push_back(head->true_branch);
                    seen.insert(head->true_branch->x86_addr);
            }
            if(head->false_branch != nullptr && seen.find(head->false_branch->x86_addr) == seen.end())
            {
                    queue.push_back(head->false_branch);
                    seen.insert(head->false_branch->x86_addr);
            }
            cur_head += 1;
            count -= 1;
        }
    }
}

void DisassmblePrinterVistor::print_one_insn(LoongArchInsInfo& insn)
{
      if(insn.opc != OPC_INVALID)
      {
           int format = insn2format[insn.opc];
           auto print_func = print_func_table[format];
           (*print_func)(std::cout, &insn);
      }
      else
          std::cout<<"Invalid insn\n";
}

void DisassmblePrinterVistor::visit(TB& tb)
{
   std::cout<<"print disassmble at x86_pc: 0x"<<std::hex<<tb.x86_addr<<std::endl;
   u_int32_t rel_cur = 0;
   u_int64_t i = 0;

   for(auto iter = tb.dis_insns.begin(); iter != tb.dis_insns.end();)
   {
        if(i == ( (tb.rels[rel_cur].tc_offset) / 4) )
        {
            int count = tb.rels[rel_cur].rel_slots_num;
            std::cout<<rel_kind2name[tb.rels[rel_cur].kind]<<std::endl;
            std::cout<<"{"<<std::endl;
            while(count > 0)
            {
                print_one_insn(*(iter->data));
                iter = iter->next;
                i+=1;
                count -= 1;
            } 
            std::cout<<"}"<<std::endl;
            rel_cur += 1;
        }
        else if(tb.origin_aot_tb->jmp_reset_offsets[0] != TB_JMP_RESET_OFFSET_INVALID 
               && i == (tb.origin_aot_tb->jmp_target_arg[0] / 4))
        {
            std::cout<<"false_branch"<<std::endl;
            std::cout<<"{"<<std::endl;
            print_one_insn(*iter->data); 
            i += 1;
            iter = iter->next;
            std::cout<<"}"<<std::endl;
        }
        else if(tb.origin_aot_tb->jmp_reset_offsets[1] != TB_JMP_RESET_OFFSET_INVALID
               && i == (tb.origin_aot_tb->jmp_target_arg[1] / 4))
        {
            std::cout<<"true_branch"<<std::endl;
            std::cout<<"{"<<std::endl;
            print_one_insn(*iter->data); 
            i += 1;
            iter = iter->next;
            std::cout<<"}"<<std::endl;
        }
        else
        {
            print_one_insn(*iter->data); 
            i += 1;
            iter = iter->next;
        }
   }
}

void DisassmbleVistor::visit(TB& tb)
{
    u_int32_t* insn_ptr = (u_int32_t*)tb.code;
    u_int32_t count = tb.code_size>>2;
    u_int32_t insn;
    while(count > 0)
    {
        LoongArchInsInfo* res = new LoongArchInsInfo();
        memset(res, 0,  sizeof(LoongArchInsInfo));
        insn = *insn_ptr;
        if(!decode(res, insn))
        {
            std::cerr<<"disassmble insn 0x" << std::hex << insn <<" failed"<<std::endl;
            res->opc = OPC_INVALID;
        }
        tb.dis_insns.insert_at_tail(res);
        insn_ptr += 1;
        count -= 1;
    }
}

void DisassmbleVistor::start(Segment& seg)
{
    for(auto tb_ptr : seg._tbs)
            (*tb_ptr).visit(*this);
}

