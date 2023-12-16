#pragma once

#include <vector>
#include <sstream>
#include <iostream>

#include "binbuf.hpp"
#include "instruction.hpp"
#include "section.hpp"

class InstructionSet : public section{

private:
    //class for a set of instruction 
    std::vector<Instruction> set;


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
        std::ostream bins = section::binstream();
        bins.write((char*)&code,sizeof(uint32_t));
        section::size() += sizeof(uint32_t);
    }

    //return the machine code memory space address
    // const binbuf& content(){
        
    //     std::ostream out(&buf);
    //     //buffer must be flush before return
    //     //else it will get problem when copy
    //     out.flush();
    //     return buf;
    // }
    
};