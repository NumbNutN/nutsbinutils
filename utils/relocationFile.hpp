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

    relocation_file() : elf(ET_REL){

    }

    void arange(){

        //insert a section string table
        uint32_t off = allocoffset(shstrtbl.size());
        shstrtbl.setOffset(off);

        //the section string table idx
        _ehdr.e_shstrndx = _ehdr.e_shnum;
        //push section string table section to section table
        insert(shstrtbl);

        //arange for program header table
        _ehdr.e_shoff += getcuroffset();
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

    friend std::ostream &operator<<(std::ostream& output,const relocation_file &relo);
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

inline std::ostream &operator<<(std::ostream& output,const relocation_file &relo){

    //first output elf header
    output << relo.base;

    //write section header table & sections
    //write section header table
    output.seekp(relo._ehdr.e_shoff, std::ios::beg);
    for(const section& sec:relo.sectionUnitList)
        output.write(reinterpret_cast<const char*>(&sec.getHeader()), sizeof(Elf32_Shdr));
    for(const section& sec:relo.sectionUnitList){
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

    input.seekg(relo._ehdr.e_shoff, std::ios::beg);
    for(int i=0;i<relo._ehdr.e_shnum;++i){
        
        //create a new section object
        Elf32_Shdr shdr;
        input.get((char*)&shdr, sizeof(Elf32_Shdr));
        section sec(shdr);
        relo.sectionUnitList.push_back(sec);

        //read the section content
        input >> sec;
    }
    //all done 
    return input;
}