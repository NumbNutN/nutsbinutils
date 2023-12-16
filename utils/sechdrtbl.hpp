#pragma once

#include <elf.h>

#include <unistd.h>

#include <fstream>
#include <iostream>
#include <vector>
#include <memory>

#include "binbuf.hpp"

class shdrtbl{

public:

    struct unit{
        Elf32_Shdr shdr;
        binbuf dat;
    };
private:

    Elf32_Ehdr& _ehdr;

    //section header table
    std::vector<unit> sectionUnitList;

public:

    shdrtbl(Elf32_Ehdr& ehdr):_ehdr(ehdr){}

    void insert(const binbuf& buf,uint32_t addr,uint32_t offset){
        
        Elf32_Shdr sechdr = {
            .sh_type = SHT_PROGBITS,
            .sh_flags = SHF_WRITE | SHF_ALLOC | SHF_EXECINSTR,
            .sh_addr = addr,
            .sh_offset = offset,
            .sh_size = (uint32_t)buf.length()
        };

        sectionUnitList.push_back({
            sechdr,
            buf
        });

        _ehdr.e_shnum++;
    }

    //write section header list
    friend std::ostream &operator<<(std::ostream& out,const shdrtbl& stbl);
};

inline std::ostream &operator<<(std::ostream& out,const shdrtbl& stbl){

    //get the last section offset
    uint32_t shdr_off = stbl.sectionUnitList.back().shdr.sh_offset + 0x100;
    stbl._ehdr.e_shoff = shdr_off;

    for(const shdrtbl::unit& obj :stbl.sectionUnitList){
        //write section header table
        out.seekp(stbl._ehdr.e_shoff, std::ios::beg);
        out.write(reinterpret_cast<const char*>(&obj.shdr), sizeof(Elf32_Shdr));

        //write each section
        char c;
        size_t len = obj.dat.length();
        std::istream in((std::streambuf*)&obj.dat);
        out.seekp(obj.shdr.sh_offset, std::ios::beg);
        while(in.get(c)){
            out.write(&c, 1);
        }
    }
    
    return out;
}