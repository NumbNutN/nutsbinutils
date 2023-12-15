#pragma once

#include <elf.h>

#include <unistd.h>

#include <fstream>
#include <vector>
#include <memory>

class shdrtbl{

public:

    struct unit{
        Elf32_Shdr shdr;
        std::unique_ptr<char> dat;
    };
private:

    Elf32_Ehdr& _ehdr;

    //section header table
    std::vector<unit> sectionUnitList;

public:

    shdrtbl(Elf32_Ehdr& ehdr):_ehdr(ehdr){}

    void insert(char* dat,uint32_t addr,uint32_t offset,uint32_t size){
        
        Elf32_Shdr sechdr = {
            .sh_type = SHT_PROGBITS,
            .sh_flags = SHF_WRITE | SHF_ALLOC | SHF_EXECINSTR,
            .sh_addr = addr,
            .sh_offset = offset,
            .sh_size = size
        };

        sectionUnitList.push_back({
            sechdr,
            (std::unique_ptr<char>)dat
        });
        _ehdr.e_shnum++;
    }

    //write section header list
    friend std::ostream &operator<<(std::ostream& out,const shdrtbl& stbl);
};

inline std::ostream &operator<<(std::ostream& out,const shdrtbl& stbl){

    //get the last section offset
    uint32_t shdr_off = stbl.sectionUnitList.back().shdr.sh_offset + 4096;
    stbl._ehdr.e_shoff = shdr_off;

    for(const shdrtbl::unit& obj :stbl.sectionUnitList){
        //write section header table
        out.seekp(stbl._ehdr.e_shoff, std::ios::beg);
        out.write(reinterpret_cast<const char*>(&obj.shdr), sizeof(Elf32_Shdr));

        //write each section
        out.seekp(obj.shdr.sh_offset, std::ios::beg);
        out.write(obj.dat.get(), obj.shdr.sh_size);
    }
    
}