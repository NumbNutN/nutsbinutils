#include "section.hpp"
#include "segment.hpp"
#include "relocatable.hpp"
#include "executable.hpp"

#include <unistd.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>

Elf32_Word secFlag2ProFlag(Elf32_Word secFlag){
    Elf32_Word res = 0;
    if((secFlag & SHF_WRITE) == SHF_WRITE)res |= PF_W;
    if((secFlag & SHF_EXECINSTR) == SHF_EXECINSTR)res |= PF_X;
    if((secFlag & SHF_ALLOC) == SHF_ALLOC)res |= PF_R;
    return res;
}

Elf32_Word secType2ProType(Elf32_Word secType){
    switch(secType){
        case SHT_PROGBITS:
            return PT_LOAD;
    }
}

/**
 * syntax:  myld {ouput}  {input}+
*/
int main(int argc,char* argv[]){

    //read the output path
    char* outputPath = argv[1];
    std::ofstream out;
    out.open(outputPath,std::ios::out | std::ios::binary);

    //create a excutable file object
    Executable exec_obj(size2shift(sysconf(_SC_PAGE_SIZE)));

    std::vector<Relocatable> reloVec;
    //read all the allocable file
    for(int i = 2;i < argc;i++){
        char* ptr = argv[i];
        //read all the relocable files
        std::ifstream fin;
        fin.open(ptr,std::ios::in | std::ios::binary);

        Relocatable frelo;
        fin >> frelo;
        //push to relocable files vector
        reloVec.push_back(frelo);
    }

    // <flags,vector<section>> 
    std::unordered_map<Elf32_Word, std::vector<Section>> map = {
        {SHF_ALLOC,std::vector<Section>()},
        {SHF_ALLOC | SHF_WRITE,std::vector<Section>()},
        {SHF_ALLOC | SHF_EXECINSTR,std::vector<Section>()},
        {SHF_ALLOC | SHF_WRITE | SHF_EXECINSTR,std::vector<Section>()}
    };

    uint32_t pos;
    //set the entry if see the _start
    for(Relocatable& relo:reloVec){
        //if a global symbol name "_start" here
        if ((pos = relo.getSymbolPos("_start")) != -1){
            exec_obj.entry() = pos;
        }
    }

    //analyse the section with same flags
    for(Relocatable& relo:reloVec){
        for(Section& sec:relo.sectionUnitList){
            if(sec.getHeader().sh_type == SHT_PROGBITS)
                map.at(sec.flags()).push_back(sec);
        }
    }

    //combine the file
    for(auto unit:map){
        
        Elf32_Word flags = secFlag2ProFlag(unit.first);
        
        //judge if there's section with that flag
        if(unit.second.empty())continue;

        //create a program header for section sets with this flag
        Segment seg(PT_LOAD,0x0,0x0,flags,size2shift(sysconf(_SC_PAGE_SIZE)));

        //insert all the section content to segment
        for(Section& sec:unit.second){
            seg.insert(sec);
        }

        //linking the section before segment insert to exec
        seg.link();

        exec_obj.insert(seg);
    }

    //arange the executable file
    exec_obj.arange();
    
    //write the executable file
    out << exec_obj;
    out.close();
}