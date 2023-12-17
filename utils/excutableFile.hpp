#include <elf.h>

#include <vector>

#include "elf.hpp"
#include "segment.hpp"

class exculate_file : public elf{

private:

    std::vector<segment> segmentUnitList; /* segement header table */

    elf& base = (elf&)*this;
public:

    exculate_file() :elf(ET_EXEC){}

    void insert(segment& seg){

        //select a new offset
        uint32_t off = elf::allocoffset(seg.size());
        seg.setOffset(off);

        //push to program table
        segmentUnitList.push_back(seg);
    }

    friend std::ostream &operator<<(std::ostream& output,const exculate_file& exec);

};

std::ostream &operator<<(std::ostream& output,const exculate_file& exec){

    output << exec.base;
    //write segment header table & sections
    //write segment header table
    output.seekp(exec._ehdr.e_phoff, std::ios::beg);
    for(const segment& seg:exec.segmentUnitList)
        output.write(reinterpret_cast<const char*>(&seg.getHeader()), sizeof(Elf32_Shdr));
    for(const segment& seg:exec.segmentUnitList){
        //write each program
        output.seekp(seg.getHeader().p_offset,std::ios::beg);
        output << seg;
    }
    return output;
}