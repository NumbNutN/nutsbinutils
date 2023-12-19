#pragma once

#include <elf.h>
#include <memory.h>

#include "section.hpp"

#include "utils.h"

class elf{

private:
    uint32_t _poff = 0x100;

protected:
    Elf32_Ehdr _ehdr;        /* ELF header */

    elf(uint16_t etype) : 
        //initialize elf header
        _ehdr({
            .e_type = etype,
            .e_machine = EM_ARM,
            .e_version = EV_CURRENT,
            .e_entry = 0x0,
            .e_shoff = 0x0,
            .e_flags = 0x5000200,
            .e_ehsize = sizeof(Elf32_Ehdr),
            .e_phentsize = sizeof(Elf32_Phdr),
            .e_phnum = 0,
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
    
    uint32_t allocoffset(uint32_t size){
        uint32_t tmp = _poff;
        uint32_t map_sz = MOD(size,3)?(ROUND(size,3)+(1<<3)):size;
        _poff += map_sz;
        return tmp;
    }

    uint32_t getcuroffset() const {
        return _poff;
    }
    
    friend std::ostream &operator<<(std::ostream& output,const elf &elf_struct);
    friend std::istream &operator>>(std::istream& input,elf& elfobj);

public:

};


inline std::ostream &operator<<(std::ostream& output,const elf &elf_struct){

    //write elf header
    output.seekp(0x0, std::ios::beg);
    output.write(reinterpret_cast<const char*>(&elf_struct._ehdr), sizeof(Elf32_Ehdr));
    //flush the content so we see the content at once it put into the output stream
    output.flush();
    return output;
}

inline std::istream &operator>>(std::istream& input,elf& elfobj){

    //read elf header
    //elf header always on the offset 0x0
    input.seekg(0x0, std::ios::beg);
    input.get((char*)&elfobj._ehdr, sizeof(Elf32_Ehdr));
    return input;
}
