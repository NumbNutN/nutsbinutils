#pragma once
#include <elf.h>

#include <vector>

#include "elf.hpp"
#include "segment.hpp"

class Executable : public elf{

private:

    Sequence segment_hdr_tbl;
    std::vector<Segment> segmentUnitList; /* segement header table */

public:

    Executable() :elf(ET_EXEC){}

    void insert(Segment& seg){

        //push to program table
        segmentUnitList.push_back(seg);

        //add segment number
        _ehdr.e_phnum += 1;

        (Container<ELF_ALIGN>&)*this << (Sequence&)segmentUnitList.back();
        
        Segment& seg2 = segmentUnitList.back();

        segment_hdr_tbl << seg2.getHeader();

        //refresh the section (all relo rewrite to section)
        for(auto psec:seg2.cus_sec_list){
            psec->refreshAll();
        }
        //refresh the seg (all section rewrite to segment)
        seg2.refreshAll();
        
    }

    void organize(){

        //for each section in a segment
        //refresh their relocatable entry

        //write the segment header table
        *this << segment_hdr_tbl;

        //arange for program header table
        _ehdr.e_phoff = get_cur_offset();
        //write the elf header
        std::ostream output(&buffer());
        output.seekp(0x0, std::ios::beg);
        output.write(reinterpret_cast<const char*>(&_ehdr), sizeof(Elf32_Ehdr));
        std::cout << (*this).buffer();
    }

    void setEntry(){
        for(auto pseg : segmentUnitList){
            for(auto psec: pseg.cus_sec_list){
                for(auto sym:psec->symbol_set){
                    if(sym->_name == "_start"){
                        _ehdr.e_entry = sym->pos();
                    }
                }
            }
        }
    }

};