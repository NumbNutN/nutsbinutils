#include "section.hpp"
#include "customizable_section.hpp"
#include "relocation_entry.hpp"

#include "utils.h"

#include <string>
#include <elf.h>
class ReloSection: public Section{

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

    std::vector<Rel<R_ARM_ABS32>> getRel(uint32_t symtab_idx){\
        std::vector<Rel<R_ARM_ABS32>>rel_list;
        std::istream in(&buffer());
        Elf32_Rel rel_tbl[size()/sizeof(Elf32_Rel)];
        in.read((char*)rel_tbl,size()/sizeof(Elf32_Rel));
        for(int i=0;i<size()/sizeof(Elf32_Rel);++i){
            if((rel_tbl[i].r_info)>>8 == symtab_idx){
                Rel<R_ARM_ABS32> rel_item;
                rel_item.set_offset(rel_tbl[i].r_offset - section.offset());
                rel_item.set_base(section.base());

                rel_list.push_back(rel_item);
            }
        }
    }

 
};