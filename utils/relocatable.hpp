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

    //symbol table
    Symtab symtbl;

    //section string table
    strtbl shstrtbl;

    //string table
    strtbl strtbl;
   
    //wait for setting to global symbol
    std::vector<std::string> global_sym_list;

public:

    //customizatable section header table
    //customizatable section has the idx same as its idx in vector
    std::vector<CustomizableSection> sectionUnitList;

    Relocatable() : elf(ET_REL){}

    void arange(){
        
        size_t idx = 0;
        //to find out the definition of the symbol
        for(std::string& sym : global_sym_list){

            //for each customizable section
            for(const CustomizableSection& sec:sectionUnitList){
                
                //if symbol exist
                uint32_t symbol_off;
                if((symbol_off = sec.getSymbolOff(sym)) != -1){

                    //add into the string table
                    uint32_t ndx = strtbl.insert(sym);
                    //add into the symbol table
                    symtbl.insert(STB_GLOBAL, ndx, symbol_off, idx);
                }
                idx++;
            }
        }

        //insert the symbol table to relocatable
        insert(symtbl);
        
        //insert the string table to relocatable
        insert(strtbl);

        //the section string table idx
        //section string table should be the last section
        _ehdr.e_shstrndx = _ehdr.e_shnum;

        //insert the section string table to relocatable
        insert(shstrtbl);

        //arange for section header table
        _ehdr.e_shoff =allocoffset(sizeof(Elf32_Shdr) * sectionUnitList.size() + 3,3);
    }

    void insert(Section& sec){
        //select a new offset
        uint32_t offset = allocoffset(sec.size(),3);
        sec.setOffset(offset);

        //also insert the section name to shstrtbl
        uint32_t idx = shstrtbl.insert(sec.getName());
        sec.setNameIdx(idx);

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

        //only record the symbol,actually add into symbol table when arange
        global_sym_list.push_back(sym);
        
    }

    friend std::ostream &operator<<(std::ostream& output,Relocatable &relo);
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

};

inline std::ostream &operator<<(std::ostream& output,Relocatable &relo){

    //first output elf header
    output << relo.base;

    //write section header table & sections

    //write section header table
    output.seekp(relo._ehdr.e_shoff, std::ios::beg);
    for(const Section& sec:relo.sectionUnitList){
        output.write(reinterpret_cast<const char*>(&sec.getHeader()), sizeof(Elf32_Shdr));
        output.flush();
    }

    //write symbol table
    output.seekp(relo.symtbl.getHeader().sh_offset);
    output << relo.symtbl;

    //write string table
    output.seekp(relo.strtbl.getHeader().sh_offset);
    output << relo.strtbl;

    //write section string table
    output.seekp(relo.shstrtbl.getHeader().sh_offset);
    output << relo.shstrtbl;

    //write each customizatable section
    for(Section& sec:relo.sectionUnitList){
        output.seekp(sec.getHeader().sh_offset,std::ios::beg);
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

    //read the section header table first
    Elf32_Word shoff = relo._ehdr.e_shoff;
    Elf32_Shdr shstrtblhdr;
    input.seekg(shoff + relo._ehdr.e_shstrndx * sizeof(Elf32_Shdr), std::ios::beg);
    input.read((char*)&shstrtblhdr, sizeof(Elf32_Shdr));

    strtbl shstrtbl(shstrtblhdr);
    input.seekg(shstrtblhdr.sh_offset);
    input >> shstrtbl;

    //read the customizatable section
    for(idx=0;idx<relo._ehdr.e_shnum - 3;++idx){
        
        //create a new section object
        Elf32_Shdr shdr;

        //read the section header
        input.seekg(relo._ehdr.e_shoff + idx*sizeof(Elf32_Shdr), std::ios::beg);
        input.read((char*)&shdr, sizeof(Elf32_Shdr));

        //read the name of section
        std::string name = shstrtbl.getName(shdr.sh_name);

        //construct a section
        CustomizableSection sec(name,shdr);

        //read the section content
        input.seekg(shdr.sh_offset);
        input >> sec;
        relo.sectionUnitList.push_back(sec);
    }

    Elf32_Shdr shdr;
    //construct the symbol table
    input.seekg(relo._ehdr.e_shoff + (idx++)*sizeof(Elf32_Shdr));
    input.read((char*)&shdr, sizeof(Elf32_Shdr));
    relo.symtbl.setHeader(shdr);
    input >> relo.symtbl;

    //construct the string table
    input.seekg(relo._ehdr.e_shoff + (idx++)*sizeof(Elf32_Shdr));
    input.read((char*)&shdr, sizeof(Elf32_Shdr));
    relo.strtbl.setHeader(shdr);
    input >> relo.strtbl;

    //construct the section string table
    input.seekg(relo._ehdr.e_shoff + (idx++)*sizeof(Elf32_Shdr));
    input.read((char*)&shdr, sizeof(Elf32_Shdr));
    relo.strtbl.setHeader(shdr);
    input >> relo.shstrtbl;

    //all done
    return input;
}


