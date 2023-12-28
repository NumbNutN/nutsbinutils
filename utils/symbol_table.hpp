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
    Symtab(const elf& elfbase) : Section(elfbase,".symtab",SHT_SYMTAB,0x0){}

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
            .st_value = sym.pos(),
            .st_size = sym.size(),
            .st_info = sym._type,
            .st_shndx = secInc
        };
        *this << symhdr;
        return _symbol_num++;
    }

    //TODO strtbl
    Symbol getSymbol(uint32_t idx){
        std::istream in(&buffer());
        Elf32_Sym symhdr;
        in.seekg(idx*sizeof(Elf32_Sym));
        in.read((char*)&symhdr,sizeof(Elf32_Sym));
        Symbol sym("temp",symhdr.st_info);
        sym.set_offset(symhdr.st_value);
        return sym;
    }

    std::vector<Symbol> get_sec_Symbol(uint32_t ndx){
        std::vector<Symbol> sym_list;
        std::istream in(&buffer());
        Elf32_Sym symhdr[size()/sizeof(Elf32_Sym)];
        in.read((char*)symhdr,size()/sizeof(Elf32_Sym));
        for(int i=0;i<size()/sizeof(Elf32_Sym);++i){
            if(symhdr[i].st_shndx == ndx){
                Symbol sym("temp",symhdr[i].st_info);
                sym.set_offset(symhdr[i].st_value);
                sym_list.push_back(sym);
            }
        }
        return sym_list;
    }

    // Elf32_Sym getSymbol(uint32_t idx){
    //     std::istream in(&buffer());
    //     Elf32_Sym sym;
    //     in.seekg(idx*sizeof(Elf32_Sym));
    //     in.read((char*)&sym,sizeof(Elf32_Sym));
    //     return sym;
    // }

    uint8_t symbolNum(){
        return _symbol_num;
    }

    friend std::ofstream& operator<<(std::ofstream& out,Symtab& strtbl);
    friend std::ifstream& operator>>(std::ifstream& in,Symtab& symtab);
};

inline std::ofstream& operator<<(std::ofstream& out,Symtab& symtab){
    symtab.setNdx(symtab.getElfbase().getShStrTblNdx() - 2);
    out << (Section&)symtab;
    return out;
}

inline std::ifstream& operator>>(std::ifstream& in,Symtab& symtab){
    symtab.setNdx(symtab.getElfbase().getShStrTblNdx() - 2);
    in >> (Section&)symtab;
    symtab._symbol_num = symtab.size() / sizeof(Elf32_Sym);
    return in;
}