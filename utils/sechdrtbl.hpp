#pragma once

#include <elf.h>

#include <unistd.h>

#include <fstream>
#include <iostream>
#include <vector>
#include <memory>

#include "binbuf.hpp"
#include "strtbl.hpp"
#include "section.hpp"

class shdrtbl{

public:

private:

    Elf32_Ehdr& _ehdr;

    //section string table
    strtbl shstrtbl;

    //section header table
    std::vector<section> sectionUnitList;

public:

    shdrtbl(Elf32_Ehdr& ehdr):_ehdr(ehdr){

        //insert a section string table
        shstrtbl.setOffset(0x300);
        insert(shstrtbl);
    }

    void insert(section& sec){
        
        //also insert the section name to shstrtbl
        uint32_t idx = shstrtbl.insert(sec.getName());
        sec.setNameIdx(idx);

        //push to section table
        sectionUnitList.push_back(sec);

        _ehdr.e_shnum++;
    }

    //write section header list
    friend std::ostream &operator<<(std::ostream& out,const shdrtbl& stbl);
};

inline std::ostream &operator<<(std::ostream& out,const shdrtbl& stbl){

    //get the last section offset
    uint32_t shdr_off = stbl.sectionUnitList.back().getSectionHeader().sh_offset + 0x100;
    stbl._ehdr.e_shoff = shdr_off;

    for(const section& sec:stbl.sectionUnitList){
        //write section header table
        out.seekp(stbl._ehdr.e_shoff, std::ios::beg);
        out.write(reinterpret_cast<const char*>(&sec.getSectionHeader()), sizeof(Elf32_Shdr));

        //write each section
        out << sec;
    }
    
    return out;
}