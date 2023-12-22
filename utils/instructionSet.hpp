#pragma once

#include <vector>
#include <sstream>
#include <iostream>

#include "binbuf.hpp"
#include "instruction.hpp"
#include "directive.hpp"
#include "section.hpp"

class InstructionSet : public section{

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

    section& base = (section&)*this;

public:
    InstructionSet() :
        section(".text",
                SHT_PROGBITS,
                0x0,
                SHF_ALLOC | SHF_EXECINSTR){
            
        }
    
    void insert(const Instruction<COMPLETE_INS>& ins){

        //write the encode to buffer
        uint32_t code = ins.encode();
        base << code;
        section::size() += sizeof(uint32_t);
    }

    void insert(const Instruction<INCOMPLETE_INS>& ins,const std::string sym){

        //add to the incomplete set
        reloInsSet.push_back({ins,sym,size()});
        //write the encode to buffer
        uint32_t code = ins.encode();
        base << code;
        section::size() += sizeof(uint32_t);
    }

    template <directive_type type>
    void insert(directive<type>& obj){
        base << obj;
        size() += obj.size();
    }

    //insert a symbol
    //a symbol use on space
    void insert(const std::string& str){
        symSet[str] = size();
    }

    uint32_t getSymbolOff(const std::string& str){
        return symSet[str];
    }

    //if symbol is defined and used within one section
    //it can be identified
    void incompleteIdentified(){

        //create a literal pool, below the section
        uint32_t curLiteralPoolPos = size();

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
                std::ostream out(&buffer());
                out.seekp(curLiteralPoolPos);
                base << symPos;
                //in armv7 architecture, PC should be fixed by mines 8
                Operand<Off> off(curLiteralPoolPos - insPos - 8);
                curLiteralPoolPos += 4;

                //reconstructor a complete one, and remove the incomplete one
                incomplete_ins.setOff(off);
                //write the new complete instruction into buffer
                out.seekp(insPos);
                base << incomplete_ins;
                
                //now delete the incomplete instruction
                it = reloInsSet.erase(it);
            }
            else{
                it++;
            }

        }
    }
    
};