#pragma once

#include <vector>
#include <sstream>
#include <iostream>

#include "binbuf.hpp"
#include "instruction.hpp"
#include "directive.hpp"
#include "section.hpp"

class CustomizableSection : public Section{

protected:

    //symbol set
    //record <symbol,abosulte position>
    std::unordered_map<std::string, uint32_t> symSet;

    //record <position,symbol used instruction>
    struct reloIns_type{
        Instruction<INCOMPLETE_INS> ins;
        std::string sym;
        uint32_t pos;
    };

    std::vector<reloIns_type> reloInsSet;

    CustomizableSection& base = (CustomizableSection&)*this;

public:
    CustomizableSection(const elf& elfbase) :
        Section(elfbase,
                ".text",
                SHT_PROGBITS,
                0x0,
                SHF_ALLOC | SHF_EXECINSTR){}
    
    void insert(const Instruction<COMPLETE_INS>& ins){

        //write the encode to buffer
        uint32_t code = ins.encode();
        base << code;
        size() += sizeof(uint32_t);
    }

    void insert(const Instruction<INCOMPLETE_INS>& ins,const std::string sym){

        //add to the incomplete set
        reloInsSet.push_back({ins,sym,size()});
        //write the encode to buffer
        uint32_t code = ins.encode();
        base << code;
        size() += sizeof(uint32_t);
    }

    template <directive_type type>
    void insert(Directive<type>& obj){
        base << obj;
        size() += obj.size();
    }

    //insert a symbol
    //a symbol use on space
    void insert(const std::string& str){
        symSet[str] = size();
    }

    //get the symbol address from a section
    //if the symbol definition is not existed, return -1
    uint32_t getSymbolOff(const std::string& str) {
        if(symSet.find(str) != symSet.end())
            return symSet[str];
        else
            return -1;
    }

    //if symbol is defined and used within one section
    //it can be identified
    void incompleteIdentified(){

        //create a literal pool, below the section
        uint32_t curLiteralPoolPos = size();

        std::ostream out(&buffer());
        //for each incomplete instruction, find if symbol is defined within one section
        for(std::vector<reloIns_type>::iterator it = reloInsSet.begin();it!=reloInsSet.end();){
            //get the symbol need to be addressed
            
            const reloIns_type& unit = *it;
            const std::string& sym = unit.sym;
            const uint32_t insPos = unit.pos;

            //get the incomlete instruction
            Instruction<INCOMPLETE_INS> incomplete_ins = unit.ins;

            //see if the symbol is existed in the same section
            if(symSet.find(sym) != symSet.end()){
                uint32_t symPos = symSet[sym];

                //write the absolute address into literal pool
                out.seekp(curLiteralPoolPos,std::ios::beg);
                out.write((char*)&symPos,sizeof(uint32_t));

                //in armv7 architecture, PC should be fixed by mines 8
                Operand<Off> off(curLiteralPoolPos - insPos - 8);
                curLiteralPoolPos += 4;

                //reconstructor a complete one, and remove the incomplete one
                incomplete_ins.setOff(off);
                //write the new complete instruction into buffer
                out.seekp(insPos,std::ios::beg);
                uint32_t code = incomplete_ins.encode();
                out.write((char*)&code,sizeof(uint32_t));

                //now delete the incomplete instruction
                it = reloInsSet.erase(it);
            }
            else{
                it++;
            }

        }
        out.flush();
    }
    
};