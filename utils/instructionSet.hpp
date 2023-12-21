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
    void insert(const directive<type>& obj){
        base << obj;
        size() += obj.size();
    }    
};