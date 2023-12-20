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

    strtbl(Elf32_Shdr shdr) :section("shstrtab",shdr){}

    uint32_t insert(std::string name){
        size_t tmp = section::size();
        base << name;
        section::size() += (name.length() + 1);
        return tmp;
    }

    std::string getName(uint32_t index){
        std::istream in(&buffer());
        in.seekg(index, std::ios::beg);
        
        char tmp[1024];
        in.get(tmp, '\0');
        std::string str(tmp);
        return str;
    }
};