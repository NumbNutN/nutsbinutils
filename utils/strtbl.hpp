#include <elf.h>

#include "binbuf.hpp"
#include "section.hpp"
#include "symbol.hpp"

#include <iostream>

enum StrTblType{
    STRTBL,
    SHSTRTBL
};

class strtbl : public Section{

private:
    StrTblType _type;

protected:
    Section& base = (Section&)*this;

public:

    strtbl(const elf& elfbase,StrTblType type) : 
        Section(elfbase,(type == STRTBL)? ".strtab":".shstrtab",SHT_STRTAB,0x0),_type(type) {
    }

    uint32_t insert(const std::string& name){
        uint32_t tmp = size();
        base << name;
        return tmp;
    }

    uint32_t insert(const Symbol& sym){
        return insert(sym._name);
    }

    std::string getName(uint32_t index){
        std::istream in(&buffer());
        in.seekg(index, std::ios::beg);
        
        char tmp[1024];
        in.get(tmp, 1024, '\0');
        std::string str(tmp);
        return str;
    }

    // friend std::ofstream& operator<<(std::ofstream& out,strtbl& strtbl);
    // friend std::ifstream& operator>>(std::ifstream& in,strtbl& strtbl);
};

// inline std::ofstream& operator<<(std::ofstream& out,strtbl& strtbl){
//     if(strtbl._type == STRTBL)
//         strtbl.setNdx(strtbl.getElfbase().getShStrTblNdx() - 1);
//     else if(strtbl._type == SHSTRTBL)
//         strtbl.setNdx(strtbl.getElfbase().getShStrTblNdx());
//     out << (Section&)strtbl;
//     return out;
// }

// inline std::ifstream& operator>>(std::ifstream& in,strtbl& strtbl){
//     if(strtbl._type == STRTBL)
//         strtbl.setNdx(strtbl.getElfbase().getShStrTblNdx() - 1);
//     else if(strtbl._type == SHSTRTBL)
//         strtbl.setNdx(strtbl.getElfbase().getShStrTblNdx());
//     in >> (Section&)strtbl;
//     return in;
// }