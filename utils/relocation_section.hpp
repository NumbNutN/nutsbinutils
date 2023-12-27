#include "section.hpp"
#include "customizable_section.hpp"
#include "relocation_entry.hpp"

#include <string>

#include <elf.h>
class ReloSection: public Section<0>{

public:
    /* construct when create a new relocatable or read a relocatable */
    ReloSection(const elf& elfbase,const std::string& name) : Section(elfbase,".rel"+name,SHT_SYMTAB,0x0){

    }

    void insert(Symbol& sym,uint32_t symtab_idx){
        //scenario1:if a relocation entry is R_ARM_ABS32 set a relo
        for(Rel<R_ARM_ABS32>& rel:sym.abs_binding_table){
            Elf32_Rel rel_struct = {
                .r_offset = rel.pos(),
                .r_info = R_ARM_ABS32 | (symtab_idx << 8)
            };
            *this << rel_struct;
        }
    }

 
};