#pragma once
#include <elf.h>

#include "elf.hpp"
#include "section.hpp"
#include "binbuf.hpp"
#include "strtbl.hpp"
#include "symbol_table.hpp"
#include "customizable_section.hpp"
#include "symbol.hpp"
#include "relocation_section.hpp"
#include "utils.h"

#include <memory.h>

#include <vector>
#include <string>
#include <fstream>


class Relocatable : public elf{

private:

    elf& base = (elf&)*this;

    uint32_t cur_sec_idx = 0;
   
    //wait for setting to global symbol
    std::vector<std::string> global_sym_list;

    //the section header table
    class SectionHdrTbl :public Sequence{

    };

    SectionHdrTbl section_hdr_tbl;

public:

    //symbol table
    Symtab symtbl;    
    //string table
    strtbl string_table;
    //section string table
    strtbl shstrtbl;

    //customizatable section header table
    //customizatable section has the idx same as its idx in vector
    std::vector<CustomizableSection> cus_section_list;

    //relocation section table
    //relocation section has the idx same as its major customization section idx plus shstrtblndx
    std::vector<ReloSection> relo_section_list;

    Relocatable() : elf(ET_REL),
    symtbl(base),
    string_table(base,STRTBL),
    shstrtbl(base,SHSTRTBL){}

private:

    //fill the symbol set of section
    //call when read from relo file
    void _fill_symbol(CustomizableSection& sec){
        //search the symbol table
        for(int i=0;i<symtbl.size()/sizeof(Elf32_Sym);++i){
            Elf32_Sym symHdr = symtbl.getSymbol(i);
            if(symHdr.st_value >= sec.getOffset() && symHdr.st_value < (sec.getOffset()+sec.size())){
                std::string name = string_table.getName(symHdr.st_name);
                Symbol sym(name);
                sym.set_offset(symHdr.st_value);
                sec.gloSymbolSet.push_back(sym);
            }
        }
    }

    void set_global_symbol(){
        
        //to find out the definition of the symbol
        for(std::string& name : global_sym_list){

            //for each customizable section
            for(CustomizableSection& sec:cus_section_list){
                
                //if symbol exist
                if(sec.symbolExist(name)){
                    Symbol& sym = sec.getSymbol(name);
                    sym._type = STB_GLOBAL;
                }
            }
        }
    }

    void organize(){

        //insert the symbol table to relocatable
        *this << (Section<0>&)symtbl;

        //insert the string table to relocatable
        *this << string_table;

        //the section string table idx
        //section string table should be the last section
        _ehdr.e_shstrndx = _ehdr.e_shnum;

        //insert the section string table to relocatable
        *this << shstrtbl;

        //arange for section header table
        _ehdr.e_shoff = get_cur_offset();
    }

public:

    //insert a global symbol into relocatable
    void insertGlobalSymbol(const std::string& sym){

        //only record the symbol
        //actually add into symbol table when arange
        global_sym_list.push_back(sym);
        
    }

    template <uint32_t align2>
    friend Relocatable& operator<<(Relocatable&,Section<align2>&);

    friend Relocatable& operator<<(Relocatable& relo,CustomizableSection& sec);

    friend std::ofstream &operator<<(std::ofstream& output,Relocatable &relo);
    friend std::ifstream &operator>>(std::ifstream& input,Relocatable &relo);

};


inline Relocatable& operator<<(Relocatable& relo,CustomizableSection& sec){
    
    //add into buffer
    relo << (Section<0>&)sec;

    //add the customizable section into list
    relo.cus_section_list.push_back(sec);

    //create a relocation table if necessary
    ReloSection relotbl(relo,sec.getName());

    //get the symbol in a customization section
    for(Symbol& sym:sec.symbol_set){
        
        //add the symbol name into string table
        //get the string idx in table
        size_t string_idx = relo.string_table.insert(sym);
        //add into symbol table
        //get the idx in symbol table
        uint32_t symbol_idx = relo.symtbl.insert(sym,string_idx,relo.cur_sec_idx);

        //insert into the relocation table
        relotbl.insert(sym, symbol_idx);
    }

    //if relotbl has content, add into section table
    if(relotbl.size())
    {
        relo.relo_section_list.push_back(relotbl);
        relo << (Section<0>&)relotbl;
    }
}

template <uint32_t align>
inline Relocatable& operator<<(Relocatable& relo,Section<align>& seq){

        //first insert the section name to shstrtbl
        //remember shstrtbl will also call this api
        uint32_t idx = relo.shstrtbl.insert(seq.getName());
        seq.setNameIdx(idx);

        //write the header
        relo.section_hdr_tbl << seq.getHeader();
        relo._ehdr.e_shnum++;

        relo << (Sequence&)seq;
}

inline std::ofstream &operator<<(std::ofstream& output,Relocatable &relo){

    //first output elf header
    output << relo.base;

    //construct the symtab and strtab

    //write section header table & sections

    //write symbol table
    output << relo.symtbl;

    //write string table
    output << relo.string_table;

    //write section string table
    output << relo.shstrtbl;

    //write each customizatable section
    for(Section& sec:relo.sectionUnitList){
        output << sec;
    }

    return output;
}

/**
 * read a relocable file
*/
inline std::ifstream &operator>>(std::ifstream& input,Relocatable &relo){

    int idx = 0;
    //read elf header
    input >> relo.base;

    //read the section string table first
    input >> relo.shstrtbl;

    //read the customizatable section
    for(idx=0;idx<relo._ehdr.e_shnum - 3;++idx){
        
        //construct a section
        CustomizableSection sec = CustomizableSection(relo);
        sec.setNdx(idx);

        //read the section content
        input >> sec;
        relo.cus_section_list.push_back(sec);

        //fill the symbol set of each section
        relo._fill_symbol(sec);
    }

    //construct the symbol table
    input >> relo.symtbl;

    //construct the string table
    input >> relo.string_table;

    //all done
    return input;
}


