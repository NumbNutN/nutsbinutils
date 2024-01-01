#pragma once

#include <elf.h>
#include <memory.h>

#include <iostream>
#include <fstream>

#include "utils.h"

#include "Container.hpp"

class elf : public Container<ELF_ALIGN>{

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
            //the align requirement of architecture
        _poff = MOD(sizeof(Elf32_Ehdr),5)?ROUND(sizeof(Elf32_Ehdr),5)+(1<<5):sizeof(Elf32_Ehdr);
        _size = _poff;
    }
    friend std::ifstream &operator>>(std::ifstream& input,elf& elfobj);

public:
    elf(){}
    
    uint32_t getSecHdrBase() const {
        return _ehdr.e_shoff;
    }

    uint32_t get_sec_num() const {
        return _ehdr.e_shnum;
    }

    uint32_t getProHdrBase() const {
        return _ehdr.e_phoff;
    }

    uint32_t getShStrTblNdx() const {
        return _ehdr.e_shstrndx;
    }

    //TEMP
    uint32_t& entry(){
        return _ehdr.e_entry;
    }
};

inline std::ifstream &operator>>(std::ifstream& input,elf& elfobj){

    //read elf header
    //elf header always on the offset 0x0
    input.seekg(0x0, std::ios::beg);
    input.read((char*)&elfobj._ehdr, sizeof(Elf32_Ehdr));
    return input;
}
