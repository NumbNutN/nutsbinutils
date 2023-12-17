#include <elf.h>

#include "section.hpp"
#include "binbuf.hpp"
#include "strtbl.hpp"

#include "utils.h"

#include <memory.h>

#include <vector>
#include <string>
#include <fstream>

class relocation_file{

private:

    Elf32_Ehdr _ehdr;        /* ELF header */

    //section string table
    strtbl shstrtbl;

    //section header table
    std::vector<section> sectionUnitList;

    uint32_t poff = 0x100;
    
public:

    uint32_t allocoffset(uint32_t size){
        uint32_t tmp = poff;
        uint32_t map_sz = MOD(size,3)?(ROUND(size,3)+(1<<3)):size;
        poff += map_sz;
        return tmp;
    }

    relocation_file(uint16_t etype) : 
        //initialize elf header
        _ehdr({
            .e_type = etype,
            .e_machine = EM_ARM,
            .e_version = EV_CURRENT,
            .e_entry = 0x0,
            .e_shoff = 0x200,
            .e_flags = 0x5000200,
            .e_ehsize = sizeof(Elf32_Ehdr),
            .e_shentsize = sizeof(Elf32_Shdr),
            .e_shnum = 0,
            .e_shstrndx = 0})
    {
        //initialize section header
        char magic[] = {
            ELFMAG0,ELFMAG1,ELFMAG2,ELFMAG3,ELFCLASS32,ELFDATA2LSB,EV_CURRENT,ELFOSABI_SYSV,0
        };
        memcpy(_ehdr.e_ident,magic,sizeof(magic));
    }

    void arange(){

        //insert a section string table
        shstrtbl.setOffset(poff + 0x100);
        //the section string table idx
        _ehdr.e_shstrndx = _ehdr.e_shnum;
        //push section string table section to section table
        insert(shstrtbl);

        //arange for section header table
        _ehdr.e_shoff += poff;
    }

    void insert(section& sec){
        
        //select a new offset
        uint32_t offset = allocoffset(sec.getSize());
        sec.setOffset(offset);

        //also insert the section name to shstrtbl
        uint32_t idx = shstrtbl.insert(sec.getName());
        sec.setNameIdx(idx);

        //push to section table
        sectionUnitList.push_back(sec);

        _ehdr.e_shnum++;
    }

    friend std::ostream &operator<<(std::ostream& output,const relocation_file &elf_struct);
};

inline std::ostream &operator<<(std::ostream& output,const relocation_file &elf_struct){

    //write elf header
    output.write(reinterpret_cast<const char*>(&elf_struct._ehdr), sizeof(Elf32_Ehdr));

    //write section header table & sections
    //write section header table
    output.seekp(elf_struct._ehdr.e_shoff, std::ios::beg);
    for(const section& sec:elf_struct.sectionUnitList)
        output.write(reinterpret_cast<const char*>(&sec.getSectionHeader()), sizeof(Elf32_Shdr));
    for(const section& sec:elf_struct.sectionUnitList){
        //write each section
        output.seekp(sec.getSectionHeader().sh_offset,std::ios::beg);
        output << sec;
    }

    return output;
}