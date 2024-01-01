#include <elf.h>

#include "binbuf.hpp"
#include "section.hpp"
#include "symbol.hpp"

#include <iostream>

enum StrTblType{
    STRTBL,
    SHSTRTBL
};

class Strtbl : public Section{

private:
    StrTblType _type;

public:

    Strtbl(StrTblType type) : 
        Section((type == STRTBL)? ".strtab":".shstrtab",SHT_STRTAB,0x0),_type(type){
    }

    uint32_t insert(const std::string& name){
        uint32_t tmp = size();
        (Section&)*this << name;
        return tmp;
    }

    uint32_t insert(const Symbol& sym){
        return insert(sym._name);
    }

    std::string getName(uint32_t index){
        std::istream in(&buffer());
        in.seekg(index, std::ios::beg);
        
        char tmp[64];
        in.get(tmp, 64, '\0');
        std::string str(tmp);
        return str;
    }

};

