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
        uint32_t off = allocoffset(seg.size());
        seg.setOffset(off);

        //push to program table
        segmentUnitList.push_back(seg);

        //add segment number
        _ehdr.e_phnum += 1;
    }

    void setEntry(Elf32_Word entry){
        _ehdr.e_entry = entry;
    }

    void arange(){
        //arange for program header table
        _ehdr.e_phoff += getcuroffset();
    }

    friend std::ostream &operator<<(std::ostream& output,exculate_file& exec);

};

inline std::ostream &operator<<(std::ostream& output,exculate_file& exec){

    output << exec.base;
    // write segment header table & segments

    // write segment header table
    output.seekp(exec._ehdr.e_phoff, std::ios::beg);
    for(const segment& seg:exec.segmentUnitList){
        Elf32_Phdr phdr = seg.getHeader();
        output.write(reinterpret_cast<const char*>(&phdr), sizeof(Elf32_Phdr));
        output.flush();
    }
    //write each program
    for(segment& seg:exec.segmentUnitList){
        output.seekp(seg.getHeader().p_offset,std::ios::beg);
        output << seg;
    }

    // for(auto it = exec.segmentUnitList.begin();it!=exec.segmentUnitList.end();++it){
    //     output.seekp(it->getHeader().p_offset,std::ios::beg);
    //     output << (*it);
    // }

    // for(std::vector<segment>::iterator it = exec.segmentUnitList.begin();it!=exec.segmentUnitList.end();++it){
    //     output.seekp((*it).getHeader().p_offset,std::ios::beg);
    //     output << (*it);
    // }
    return output;
}