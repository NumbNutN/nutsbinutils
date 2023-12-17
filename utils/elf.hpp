#include <elf.h>
#include <memory.h>

#include "section.hpp"

#include "utils.h"

class elf{

private:
    uint32_t poff = 0x100;


protected:
    Elf32_Ehdr _ehdr;        /* ELF header */

    elf(uint16_t etype) : 
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
    
    uint32_t allocoffset(uint32_t size){
        uint32_t tmp = poff;
        uint32_t map_sz = MOD(size,3)?(ROUND(size,3)+(1<<3)):size;
        poff += map_sz;
        return tmp;
    }
    
    friend std::ostream &operator<<(std::ostream& output,const elf &elf_struct);
};


inline std::ostream &operator<<(std::ostream& output,const elf &elf_struct){

    //write elf header
    output.write(reinterpret_cast<const char*>(&elf_struct._ehdr), sizeof(Elf32_Ehdr));
    return output;
}