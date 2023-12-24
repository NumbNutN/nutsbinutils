#include "section.hpp"

#include <vector>
#include <string>

class Symtab : public Section{

private:
    //store all the symbol
    std::vector<Elf32_Sym> symbolList;

    Section& base = (Section&)*this;

public:

    Symtab() : Section(".symtab",SHT_SYMTAB,0x0){}

    Symtab(const Elf32_Shdr& sechdr) : Section(".symtab",sechdr){}

    //insert a symbol
    void insert(uint8_t symType,Elf32_Word symName,Elf32_Word symVal,uint8_t secInc,Elf32_Word symSize = 0){
        Elf32_Sym symhdr = {
            .st_name = symName,
            .st_value = symVal,
            .st_size = symSize,
            .st_info = symType,
            .st_shndx = secInc
        };
        symbolList.push_back(symhdr);

        //add into buffer
        base << symhdr;
        size() += sizeof(symhdr);
    }

    friend std::istream& operator>>(std::istream& in, Symtab& symtab);
};

inline std::istream& operator>>(std::istream& in, Symtab& symtab){
    
}