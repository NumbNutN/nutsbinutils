#include <elf.h>

#include "binbuf.hpp"
#include "section.hpp"

#include <iostream>

class strtbl : public Section{

protected:
    Section& base = (Section&)*this;

public:

    strtbl() : 
        Section(".shstrtab",SHT_STRTAB,0x0) {
    }

    strtbl(Elf32_Shdr shdr) :Section("shstrtab",shdr){}

    uint32_t insert(std::string name){
        size_t tmp = Section::size();
        base << name;
        Section::size() += (name.length() + 1);
        return tmp;
    }

    std::string getName(uint32_t index){
        std::istream in(&buffer());
        in.seekg(index, std::ios::beg);
        
        char tmp[1024];
        in.get(tmp, 1024, '\0');
        std::string str(tmp);
        return str;
    }
};