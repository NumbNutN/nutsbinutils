#include <elf.h>

#include "sechdrtbl.hpp"
#include "memory.h"

#include <string>
#include <fstream>
class elf{

private:

    Elf32_Ehdr ehdr;        /* ELF header */
    shdrtbl shdr;           /* section header */

    uint32_t _off_cnt = 0x0;
public:

    uint32_t new_sechdroff(){
        _off_cnt += 0x1000;
        return _off_cnt;
    }

    elf(uint16_t etype) : 
        //initialize elf header
        ehdr({
            .e_type = etype,
            .e_machine = EM_ARM,
            .e_shoff = new_sechdroff(),
            .e_shnum = 0,}),
        //initialize section header
        shdr(shdrtbl(ehdr)) {
        
        memcpy(ehdr.e_ident,"ELF",3);

    }

    void add_section(char* dat,uint32_t addr,uint32_t size){
        uint32_t offset = new_sechdroff();
        shdr.insert(dat,addr,offset,size);
    }

    friend std::ostream &operator<<(std::ostream& output,const elf &elf_struct);
};

inline std::ostream &operator<<(std::ostream& output,const elf &elf_struct){

    //write elf header
    output.write(reinterpret_cast<const char*>(&elf_struct.ehdr), sizeof(Elf32_Ehdr));

    //write section header table & sections
    output << elf_struct.shdr;
}