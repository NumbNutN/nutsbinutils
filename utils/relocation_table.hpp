#include "section.hpp"
#include "customizable_section.hpp"
#include "relocation_entry.hpp"

#include "utils.h"

#include <string>
#include <elf.h>
class Relotab: public Section{

public:

    Relotab(){}
    /* construct when create a new relocatable or read a relocatable */
    Relotab(const std::string& name) : Section(".rel"+name,SHT_RELA,0x0){}

    void insert(Symbol& sym,uint32_t symtab_idx){
        //scenario1:if a relocation entry is R_ARM_ABS32 set a relo
        for(std::shared_ptr<Rel<R_ARM_ABS32>>& rel:sym.abs_binding_table){
            Elf32_Rel rel_struct = {
                .r_offset = (uint32_t)rel->get_offset(),
                .r_info = R_ARM_ABS32 | (symtab_idx << 8)
            };
            *this << rel_struct;
        }
    }
};