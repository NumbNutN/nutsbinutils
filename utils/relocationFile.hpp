#include <elf.h>

#include "elf.hpp"
#include "section.hpp"
#include "binbuf.hpp"
#include "strtbl.hpp"

#include "utils.h"

#include <memory.h>

#include <vector>
#include <string>
#include <fstream>

class relocation_file : public elf{

private:

    elf& base = (elf&)*this;

    //section string table
    strtbl shstrtbl;

    //section header table
    std::vector<section> sectionUnitList;    

public:

    relocation_file() : elf(ET_REL){}

    void arange(){

        //insert a section string table
        uint32_t off = allocoffset(shstrtbl.size());
        shstrtbl.setOffset(off);

        //the section string table idx
        _ehdr.e_shstrndx = _ehdr.e_shnum;
        //push section string table section to section table
        insert(shstrtbl);

        //arange for section header table
        _ehdr.e_shoff =0x300;
    }

    void insert(section& sec){
        
        //select a new offset
        uint32_t offset = allocoffset(sec.size());
        sec.setOffset(offset);

        //also insert the section name to shstrtbl
        uint32_t idx = shstrtbl.insert(sec.getName());
        sec.setNameIdx(idx);

        //push to section table
        sectionUnitList.push_back(sec);

        _ehdr.e_shnum++;
    }

    friend std::ostream &operator<<(std::ostream& output,relocation_file &relo);
    friend std::istream &operator>>(std::istream& input,relocation_file &relo);

public:

    //iterator
    using iterator = typename std::vector<section>::iterator;

    //implement begin and end
    iterator begin(){
        return sectionUnitList.begin();
    }

    iterator end(){
        return sectionUnitList.end();
    }

};

inline std::ostream &operator<<(std::ostream& output,relocation_file &relo){

    //first output elf header
    output << relo.base;

    //write section header table & sections
    //write section header table
    output.seekp(relo._ehdr.e_shoff, std::ios::beg);
    for(const section& sec:relo.sectionUnitList){
        output.write(reinterpret_cast<const char*>(&sec.getHeader()), sizeof(Elf32_Shdr));
        output.flush();
    }

    for(section& sec:relo.sectionUnitList){
        //write each section
        output.seekp(sec.getHeader().sh_offset,std::ios::beg);
        output << sec;
    }
    return output;
}

/**
 * read a relocable file
*/
inline std::istream &operator>>(std::istream& input,relocation_file &relo){

    //read elf header
    input >> relo.base;

    //read the shstrtbl first
    Elf32_Word shoff = relo._ehdr.e_shoff;
    Elf32_Shdr shstrtblhdr;
    input.seekg(shoff + relo._ehdr.e_shstrndx * sizeof(Elf32_Shdr), std::ios::beg);
    input.read((char*)&shstrtblhdr, sizeof(Elf32_Shdr));
    strtbl shstrtbl(shstrtblhdr);
    input >> shstrtbl;

    for(int i=0;i<relo._ehdr.e_shnum;++i){
        
        //skip the shstrtbl
        if(i == relo._ehdr.e_shstrndx)continue;
        //create a new section object
        Elf32_Shdr shdr;

        //read the section header
        input.seekg(relo._ehdr.e_shoff + i*sizeof(Elf32_Shdr), std::ios::beg);
        input.get((char*)&shdr, sizeof(Elf32_Shdr));

        //read the name of section
        std::string name = shstrtbl.getName(shdr.sh_name);

        //construct a section
        section sec(name,shdr);

        //read the section content
        input >> sec;
        relo.sectionUnitList.push_back(sec);
    }
    //all done 
    return input;
}


