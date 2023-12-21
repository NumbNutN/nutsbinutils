#include "relocationFile.hpp"
#include "excutableFile.hpp"
#include "segment.hpp"
#include "section.hpp"

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
    exculate_file exec_obj;

    std::vector<relocation_file> reloVec;
    //read all the allocable file
    for(int i = 2;i < argc;i++){
        char* ptr = argv[i];
        //read all the relocable files
        std::ifstream fin;
        fin.open(ptr,std::ios::in | std::ios::binary);

        relocation_file frelo;
        fin >> frelo;
        //push to relocable files vector
        //reloVec.push_back(frelo);
        // std::cout << frelo;
    }

    // <flags,vector<section>> 
    std::unordered_map<Elf32_Word, std::vector<section>> map = {
        {SHF_ALLOC,std::vector<section>()},
        {SHF_ALLOC | SHF_WRITE,std::vector<section>()},
        {SHF_ALLOC | SHF_EXECINSTR,std::vector<section>()},
        {SHF_ALLOC | SHF_WRITE | SHF_EXECINSTR,std::vector<section>()}
    };

    //analyse the section with same flags
    for(relocation_file& relo:reloVec){
        for(section& sec:relo.sectionUnitList){
            // std::cout << sec;
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
        segment seg(PT_LOAD,0x0,0x0,flags);

        //insert all the section content to segment
        for(section& sec:unit.second){
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