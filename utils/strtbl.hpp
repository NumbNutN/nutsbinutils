#include <elf.h>

#include "binbuf.hpp"
#include "section.hpp"

class strtbl : public section{

protected:
    section& base = (section&)*this;

public:

    strtbl() : 
        section(".shstrtab",SHT_STRTAB,0x0) {
    }

    uint32_t insert(std::string name){
        size_t tmp = section::size();
        base << name;
        section::size() += (name.length() + 1);
        return tmp;
    }

};