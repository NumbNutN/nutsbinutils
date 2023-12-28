#pragma once
#include <elf.h>

#include <vector>

#include "elf.hpp"
#include "segment.hpp"

class Executable : public elf{

private:

    uint32_t _palign;
    elf& base = (elf&)*this;
    std::vector<Segment> segmentUnitList; /* segement header table */

public:

    Executable(uint32_t align) :elf(ET_EXEC),_palign(align){}

    void insert(Segment& seg){

        //select a new offset
        uint32_t off = allocoffset(seg.size(),_palign);
        seg.set_offset(off);

        //push to program table
        segmentUnitList.push_back(seg);

        //add segment to container management
        Container::insert(segmentUnitList.back());

        //add segment number
        _ehdr.e_phnum += 1;

        //arange for program header table
        _ehdr.e_phoff = getcuroffset();
    }

    void setEntry(Elf32_Word entry){
        _ehdr.e_entry = entry;
    }

    friend std::ostream &operator<<(std::ostream& output,Executable& exec);

};

inline std::ostream &operator<<(std::ostream& output,Executable& exec){

    output << exec.base;
    // write segment header table & segments

    // write segment header table
    output.seekp(exec._ehdr.e_phoff, std::ios::beg);
    for(const Segment& seg:exec.segmentUnitList){
        Elf32_Phdr phdr = seg.getHeader();
        output.write(reinterpret_cast<const char*>(&phdr), sizeof(Elf32_Phdr));
        output.flush();
    }
    //write each program
    for(Segment& seg:exec.segmentUnitList){
        output.seekp(seg.getHeader().p_offset,std::ios::beg);
        output << seg;
    }

    return output;
}