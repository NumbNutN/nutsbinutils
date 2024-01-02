#pragma once
#include <elf.h>

#include <vector>

#include "elf.hpp"
#include "segment.hpp"

class Executable : public elf, public Container<SEGMENT_ALIGN>{

private:

    Sequence segment_hdr_tbl;
    std::vector<Segment> segmentUnitList; /* segement header table */

public:

    Executable() :elf(ET_EXEC){
        //the align requirement of architecture
        _poff = MOD(sizeof(Elf32_Ehdr),5)?ROUND(sizeof(Elf32_Ehdr),5)+(1<<5):sizeof(Elf32_Ehdr);
        _size = _poff;        
    }

    void insert(Segment& seg){

        //push to program table
        segmentUnitList.push_back(seg);

        //add segment number
        _ehdr.e_phnum += 1;

        (Container<SEGMENT_ALIGN>&)*this << (Sequence&)segmentUnitList.back();
        std::cout << ((Sequence&)segmentUnitList.back()).buffer();
        std::cout << buffer();
        
        Segment& seg2 = segmentUnitList.back();

        segment_hdr_tbl << seg2.getHeader();

        //refresh the section (all relo rewrite to section)
        for(auto sec:seg2.cus_sec_list){
            sec.refreshAll();
        }
        //refresh the seg (all section rewrite to segment)
        seg2.refreshAll();
        
    }

    void organize(){

        //for each section in a segment
        //refresh their relocatable entry

        //arange for program header table
        _ehdr.e_phoff = get_cur_offset();

        //write the segment header table
        *this << segment_hdr_tbl;

        //write the elf header
        std::ostream output(&buffer());
        output.seekp(0x0, std::ios::beg);
        output.write(reinterpret_cast<const char*>(&_ehdr), sizeof(Elf32_Ehdr));
        std::cout << (*this).buffer();
    }

    void setEntry(){
        for(auto seg : segmentUnitList){
            for(auto sec: seg.cus_sec_list){
                for(auto sym:sec.symbol_set){
                    if(sym->_name == "_start"){
                        _ehdr.e_entry = sym->pos();
                    }
                }
            }
        }
    }

};