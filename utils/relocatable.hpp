#pragma once
#include <elf.h>

#include "elf.hpp"
#include "section.hpp"
#include "binbuf.hpp"
#include "strtbl.hpp"
#include "symbol_table.hpp"
#include "customizable_section.hpp"
#include "symbol.hpp"
#include "relocation_table.hpp"
#include "utils.h"

#include <memory.h>

#include <vector>
#include <string>
#include <fstream>
#include <memory>

extern void get_section_symbol(const Section& sec,uint32_t ndx,Symtab& symtab,Strtbl& strtbl,Relotab& relotab);

inline void get_section_symbol(CustomizableSection& sec,uint32_t ndx,Symtab& symtab,Strtbl& strtbl,Relotab& relotab){

    std::istream in(&symtab.buffer());
    size_t sym_num = symtab.size()/sizeof(Elf32_Sym);
    Elf32_Sym symhdr[sym_num];
    in.read((char*)symhdr,symtab.size());

    for(int symbol_idx=0;symbol_idx<sym_num;++symbol_idx){
        if(symhdr[symbol_idx].st_shndx == ndx){

            //look for name
            std::string name = strtbl.getName(symhdr[symbol_idx].st_name);
            //auto sym = std::make_unique<Symbol>(name,symhdr[symbol_idx].st_info)
            Symbol* sym = new Symbol(name,symhdr[symbol_idx].st_info);
            sym->set_offset(symhdr[symbol_idx].st_value);
            sec.symbol_set.push_back(std::shared_ptr<Symbol>(sym));

            //construct the relo units
            std::vector<std::shared_ptr<Rel<R_ARM_ABS32>>>rel_list;
            std::istream in(&relotab.buffer());
            size_t relo_num = relotab.size()/sizeof(Elf32_Rel);
            Elf32_Rel rel_tbl[relo_num];
            in.read((char*)rel_tbl,relotab.size());

            for(int relo_idx=0;relo_idx<relo_num;++relo_idx){

                if((rel_tbl[relo_idx].r_info)>>8 == symbol_idx){
                    std::shared_ptr<Rel<R_ARM_ABS32>> rel_item = std::shared_ptr<Rel<R_ARM_ABS32>>(new Rel<R_ARM_ABS32>);
                    rel_item->set_offset(rel_tbl[relo_idx].r_offset);
                    rel_item->set_base(sec.pos());

                    //binding the relo entry to the symbol
                    sym->bind(rel_item);

                    //add to relocatable list
                    sec.relo_abs_list.push_back(rel_item);

                    //add the relo entry to the custom section maintain list
                    ((Container<0>&)sec).insert((Sequence&)*rel_item);
                }
            }
        }
    }
}

class Relocatable : public elf,public Container<4>{

private:


    uint32_t cur_sec_idx = 0;
   
    //wait for setting to global symbol
    std::vector<std::string> global_sym_list;

    // SectionHdrTbl section_hdr_tbl;

public:

    //symbol table
    Symtab symtbl;    
    //string table
    Strtbl strtbl;
    //section string table
    Strtbl shstrtbl;

    //section header table
    Sequence section_hdr_tbl;

    //customizatable section header table
    //customizatable section has the idx same as its idx in vector
    std::vector<CustomizableSection> cus_section_list;

    //relocation section table
    //relocation section has the idx same as its major customization section idx plus shstrtblndx
    std::vector<Relotab> relo_section_list;

    Relocatable() : elf(ET_REL),
    strtbl(STRTBL),
    shstrtbl(SHSTRTBL){
        //the align requirement of architecture
        _poff = MOD(sizeof(Elf32_Ehdr),5)?ROUND(sizeof(Elf32_Ehdr),5)+(1<<5):sizeof(Elf32_Ehdr);
        _size = _poff;
    }

private:

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

public:

    //insert a global symbol into relocatable
    void insertGlobalSymbol(const std::string& sym){

        //only record the symbol
        //actually add into symbol table when arange
        global_sym_list.push_back(sym);
        
    }

    void organize(){

        //insert the symbol table to relocatable
        *this << (Section&)symtbl;

        //insert the string table to relocatable
        *this << (Section&)strtbl;

        //the section string table idx
        //section string table should be the last section
        _ehdr.e_shstrndx = _ehdr.e_shnum;

        //insert the section string table to relocatable
        *this << (Section&)shstrtbl;

        //arange for section header table
        _ehdr.e_shoff = get_cur_offset();

        //now do insert the section headers table
        *this << section_hdr_tbl;
        std::cout << section_hdr_tbl.buffer();

        //write the elf header
        std::ostream output(&buffer());
        output.seekp(0x0, std::ios::beg);
        output.write(reinterpret_cast<const char*>(&_ehdr), sizeof(Elf32_Ehdr));
        std::cout << (*this).buffer();

    }

    friend Relocatable& operator<<(Relocatable&,Section&);

    friend Relocatable& operator<<(Relocatable& relo,CustomizableSection& sec);

    friend std::ofstream &operator<<(std::ofstream& output,Relocatable &relo);
    friend std::ifstream &operator>>(std::ifstream& input,Relocatable &relo);

};


inline Relocatable& operator<<(Relocatable& relo,CustomizableSection& sec){
    
    //add into buffer
    relo << (Section&)sec;
    std::cout << relo.buffer();
    std::cout << sec.buffer();
    //add the customizable section into list
    relo.cus_section_list.push_back(sec);

    //create a relocation table if necessary
    Relotab relotbl(sec._name);

    //get the symbol in a customization section
    for(std::shared_ptr<Symbol>& sym:sec.symbol_set){
        
        //add the symbol name into string table
        //get the string idx in table
        size_t string_idx = relo.strtbl.insert(*sym);
        //add into symbol table
        //get the idx in symbol table
        uint32_t symbol_idx = relo.symtbl.insert(*sym,string_idx,relo.cur_sec_idx);

        //get all the relocatable entry connected to the symbol and insert into the relocation table
        relotbl.insert(*sym, symbol_idx);
    }

    //if relotbl has content, add into section table
    if(relotbl.size())
    {
        relo.relo_section_list.push_back(relotbl);
        relo << (Section&)relotbl;
    }
}

inline Relocatable& operator<<(Relocatable& relo,Section& seq){

        //first insert the section name to shstrtbl
        //remember shstrtbl will also call this api
        uint32_t idx = relo.shstrtbl.insert(seq._name);

        //allocate to a certain place
        relo << (Sequence&)seq;

        //construct a section header
        Elf32_Shdr shdr = {
            .sh_name = idx,
            .sh_type = seq._type,
            .sh_flags = seq._flags,
            .sh_offset = seq.pos(),
            .sh_size = seq.size()
        };

        //write the header
        relo.section_hdr_tbl << shdr;
        relo._ehdr.e_shnum++;

        return relo;

}

inline std::ofstream &operator<<(std::ofstream& output,Relocatable &relo){

    //output the relo
    (std::ostream&)output << relo;

    return output;
}


inline std::ifstream &operator>>(std::ifstream& in,Relocatable &relo){
    std::unordered_map<std::string,Relotab> relotab_list;

    //read elf header
    in.seekg(0);
    in.read((char*)&relo._ehdr,sizeof(Elf32_Ehdr));

    uint32_t sec_hdr_tbl_base = relo.getSecHdrBase();
    uint32_t sec_hdr_tbl_num = relo.get_sec_num();
    Elf32_Shdr sec_hdr_tbl[sec_hdr_tbl_num];
    
    //read the section header table
    in.seekg(relo._ehdr.e_shoff);
    in.read((char*)sec_hdr_tbl, sec_hdr_tbl_num*sizeof(Elf32_Shdr));

    //get section string table
    in.seekg(sec_hdr_tbl[relo.getShStrTblNdx()].sh_offset);
    relo.shstrtbl.read(in, sec_hdr_tbl[relo.getShStrTblNdx()].sh_size);

    for(int i=0;i<sec_hdr_tbl_num;++i){
        Elf32_Shdr hdr = sec_hdr_tbl[i];
        std::string name = relo.shstrtbl.getName(hdr.sh_name);

        in.seekg(hdr.sh_offset);

        //get string table
        if(hdr.sh_type == SHT_STRTAB && i!= relo.getShStrTblNdx()){
            relo.strtbl.read(in, hdr.sh_size);
        }

        //get symbol table
        else if(hdr.sh_type == SHT_SYMTAB){
            relo.symtbl.read(in, hdr.sh_size);
        }

        else if(hdr.sh_type == SHT_RELA){
            std::string name = relo.shstrtbl.getName(hdr.sh_name);
            Relotab relo(name);
            relo.read(in, hdr.sh_size);
            relotab_list.insert(std::unordered_map<std::string,Relotab>::value_type(name,relo));
        }
    }

    for(int i=0;i<sec_hdr_tbl_num;++i){

        Elf32_Shdr hdr = sec_hdr_tbl[i];
        //get the name
        std::string name = relo.shstrtbl.getName(hdr.sh_name);
        //seek to the section content
        in.seekg(hdr.sh_offset);

        //get other section
        if(hdr.sh_type == SHT_PROGBITS){
            relo.cus_section_list.push_back(CustomizableSection(name));
            CustomizableSection& cus_sec = relo.cus_section_list.back();
            //if the custom section own a relocatable section
            cus_sec.read(in, hdr.sh_size);

            if(relotab_list.find(".rel"+name) != relotab_list.end())
                //rewrite all the symbol and their binding relocatable entry
                get_section_symbol(cus_sec,i,relo.symtbl,relo.strtbl,relotab_list[".rel"+name]);
        }
    }
}

