#pragma once

#include <vector>
#include <sstream>
#include <iostream>

#include "binbuf.hpp"
#include "instruction.hpp"
#include "directive.hpp"
#include "section.hpp"

class InstructionSet : public section{

private:
    //class for a set of instruction 
    std::vector<Instruction> set;

protected:

    //symbol set
    std::unordered_map<std::string, uint32_t> symSet;

    section& base = (section&)*this;

public:
    InstructionSet() :
        section(".text",
                SHT_PROGBITS,
                0x0,
                SHF_ALLOC | SHF_EXECINSTR){
            
        }
    
    void insert(const Instruction& ins){

        //add the instruction to the list
        set.push_back(ins);

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
    void insert(const std::string& str){
        symSet[str] = size();
    }

    uint32_t getSymbolOff(const std::string& str){
        return symSet[str];
    }
    
};