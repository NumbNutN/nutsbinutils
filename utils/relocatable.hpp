#include <elf.h>

#include "elf.hpp"
#include "section.hpp"
#include "binbuf.hpp"
#include "strtbl.hpp"
#include "symbol_table.hpp"
#include "customizable_section.hpp"

#include "utils.h"

#include <memory.h>

#include <vector>
#include <string>
#include <fstream>

class Relocatable : public elf{

private:

    elf& base = (elf&)*this;

    uint32_t cur_sec_idx = 0;
    //symbol table
    Symtab symtbl;
    //string table
    strtbl string_table;
    //section string table
    strtbl shstrtbl;
   
    //wait for setting to global symbol
    std::vector<std::string> global_sym_list;

public:

    //customizatable section header table
    //customizatable section has the idx same as its idx in vector
    std::vector<CustomizableSection> sectionUnitList;

    Relocatable() : elf(ET_REL),
    symtbl(base),
    string_table(base,STRTBL),
    shstrtbl(base,SHSTRTBL){}

    void arange(){
        
        size_t idx = 0;
        //to find out the definition of the symbol
        for(std::string& sym : global_sym_list){

            //for each customizable section
            for(CustomizableSection& sec:sectionUnitList){
                
                //if symbol exist
                uint32_t symbol_off;
                if((symbol_off = sec.getSymbolOff(sym)) != -1){

                    //add into the string table
                    uint32_t ndx = string_table.insert(sym);
                    //add into the symbol table
                    symtbl.insert(STB_GLOBAL, ndx, symbol_off, idx);
                }
                idx++;
            }
        }

        //insert the symbol table to relocatable
        insert(symtbl);

        //insert the string table to relocatable
        insert(string_table);

        //the section string table idx
        //section string table should be the last section
        _ehdr.e_shstrndx = _ehdr.e_shnum;

        //insert the section string table to relocatable
        insert(shstrtbl);

        //arange for section header table
        _ehdr.e_shoff =allocoffset(sizeof(Elf32_Shdr) * sectionUnitList.size() + 3,3);
    }

    void insert(Section& sec){

        //first insert the section name to shstrtbl
        //remember shstrtbl will also call this api
        uint32_t idx = shstrtbl.insert(sec.getName());
        sec.setNameIdx(idx);

        //select a new offset
        uint32_t offset = allocoffset(sec.size(),3);
        sec.setOffset(offset);

        //select a ndx
        sec.setNdx(cur_sec_idx++);

        _ehdr.e_shnum++;
    }

    //insert a section into relocatable
    void insert(CustomizableSection& sec){
        
        //insert section
        insert((Section&)sec);

        //push to customizatable section table
        sectionUnitList.push_back(sec);
    }

    //insert a global symbol into relocatable
    void insert(const std::string& sym){

        //only record the symbol
        //actually add into symbol table when arange
        global_sym_list.push_back(sym);
        
    }


    friend std::ofstream &operator<<(std::ofstream& output,Relocatable &relo);
    friend std::ifstream &operator>>(std::ifstream& input,Relocatable &relo);

public:

    //iterator
    using iterator = typename std::vector<CustomizableSection>::iterator;

    //implement begin and end
    iterator begin(){
        return sectionUnitList.begin();
    }

    iterator end(){
        return sectionUnitList.end();
    }

    uint32_t getSymbolPos(const std::string& sym){
        //reserve the symbol table
        for(int i=0;i<symtbl.size()/sizeof(Elf32_Sym);++i){
            Elf32_Sym symHdr = symtbl.getSymbol(i);
            //search for name in string table
            std::string name = string_table.getName(symHdr.st_name);
            if (name == sym){
                return symHdr.st_value;
            }
        }
        return -1;
    }

};

inline std::ofstream &operator<<(std::ofstream& output,Relocatable &relo){

    //first output elf header
    output << relo.base;

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
        CustomizableSection sec(relo.base);
        sec.setNdx(idx);

        //read the section content
        input >> sec;
        relo.sectionUnitList.push_back(sec);
    }

    //construct the symbol table
    input >> relo.symtbl;

    //construct the string table
    input >> relo.string_table;

    //all done
    return input;
}


