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
    Executable exec_obj;

    std::vector<Relocatable> relocatable_list;
    //read all the allocable file
    for(int i = 2;i < argc;i++){
        char* ptr = argv[i];
        //read all the relocable files
        std::ifstream fin;
        fin.open(ptr,std::ios::in | std::ios::binary);
        //push to relocable files vector
        relocatable_list.push_back(Relocatable());
        //binding the dymanic member like symbol and incomplete instruction
        //so the relocatable object should not copy
        fin >> relocatable_list.back();
    }

    // <flags,vector<section>> 
    std::unordered_map<Elf32_Word, std::vector<std::shared_ptr<Section>>> map = {
        {SHF_ALLOC,std::vector<std::shared_ptr<Section>>()},
        {SHF_ALLOC | SHF_WRITE,std::vector<std::shared_ptr<Section>>()},
        {SHF_ALLOC | SHF_EXECINSTR,std::vector<std::shared_ptr<Section>>()},
        {SHF_ALLOC | SHF_WRITE | SHF_EXECINSTR,std::vector<std::shared_ptr<Section>>()}
    };

    std::vector<CustomizableSection*> custom_sec_list;

    uint32_t pos;
    //set the entry if see the _start
    // for(Relocatable& relo:reloVec){
    //     //if a global symbol name "_start" here
    //     if ((pos = relo.getSymbolPos("_start")) != -1){
    //         exec_obj.entry() = pos;
    //     }
    // }

    //analyse the section with same flags
    for(Relocatable& relo:relocatable_list){
        for(CustomizableSection& sec:relo.cus_section_list){
            custom_sec_list.push_back(&sec);
        }
    }
    
    //create a program header for section sets with this flag
    Segment seg(PT_LOAD,0x0,PF_X|PF_R|PF_W);
    //combine the file
    for(auto sec:custom_sec_list){
        
        Elf32_Word flags = secFlag2ProFlag(sec->_flags);
        
        //insert all the section content to segment
        seg.insert(*sec);
    }
    exec_obj.insert(seg);

    //arange the executable file
    exec_obj.setEntry();
    exec_obj.organize();

    //write the executable file
    std::cout << exec_obj.buffer();
    out << exec_obj;
    out.flush();
    out.close();
}