#include "section.hpp"
#include "symbol.hpp"

#include <vector>
#include <string>
#include <iostream>

class Symtab : public Section{

private:
    uint8_t _symbol_num = 0;
public:

    /* construct when create a new relocatable or read a relocatable */
    Symtab() : Section(".symtab",SHT_SYMTAB,0x0){}

    //insert a symbol
    void insert(uint8_t symType,Elf32_Word symName,Elf32_Word symVal,uint8_t secInc,Elf32_Word symSize = 0){
        Elf32_Sym symhdr = {
            .st_name = symName,
            .st_value = symVal,
            .st_size = symSize,
            .st_info = symType,
            .st_shndx = secInc
        };

        //add into buffer
        *this << symhdr;
        _symbol_num++;
    }

    uint32_t insert(Symbol& sym,Elf32_Word symName,uint8_t secInc){
        Elf32_Sym symhdr = {
            .st_name = symName,
            .st_value = (uint32_t)sym.get_offset(),
            .st_size = sym.size(),
            .st_info = sym._type,
            .st_shndx = secInc
        };
        *this << symhdr;
        return _symbol_num++;
    }

};


