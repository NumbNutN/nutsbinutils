#pragma once

#include <vector>
#include <sstream>
#include <iostream>

#include "binbuf.hpp"
#include "instruction.hpp"

class InstructionSet{

private:
    //class for a set of instruction 
    std::vector<Instruction> set;

    //string buffer
    binbuf buf;
    //section length
    size_t _len = 0;

public:
    InstructionSet() {}
    
    void insert(const Instruction& ins){

        std::ostream out(&buf);
        //add the instruction to the list
        set.push_back(ins);

        //write the encode to buffer
        uint32_t code = ins.encode();
        out.write((char*)&code,sizeof(uint32_t));
        _len += sizeof(uint32_t);
    }

    //return the machine code memory space address
    const binbuf& content(){
        
        std::ostream out(&buf);
        //buffer must be flush before return
        //else it will get problem when copy
        out.flush();
        return buf;
    }
    
};