#ifndef __VISTOR
#define __VISTOR
#include "./aot_class.h"
#include<set>

class TB_Vistor
{
    public:
        virtual void visit(TB& tb) = 0;
};

class TB_AddRels_Vistor: TB_Vistor
{
    const std::vector<AOT_rel>* rels;
    public:
        void visit(TB& tb);
        void start(Segment& seg, const std::vector<AOT_rel>* rels);
};

class EntryBlockVistor: TB_Vistor
{
        public:
            std::vector<u_int64_t> entrys;
            void visit(TB& tb);
            void start(Segment& seg);
};

class DisassmbleVistor: TB_Vistor
{
    public:
            void visit(TB& tb);
            void start(Segment& seg);
};

class DisassmblePrinterVistor: TB_Vistor
{
    public:
            u_int64_t seg_start;
            const std::vector<AOT_rel>* rels;
            void visit(TB& tb);
            void start(Segment& seg, const std::vector<AOT_rel>* rels);
            void start(Segment& seg, u_int64_t entry, const std::vector<AOT_rel>* rels);
    private:
            void print_one_insn(LoongArchInsInfo& insn);
};

class testVistor: TB_Vistor
{
    public:
            void visit(TB& tb);
            void start(Segment& seg);
};
#endif
