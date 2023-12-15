#pragma once

#include <vector>
#include <sstream>

#include "instruction.hpp"

class InstructionSet{

private:
    //class for a set of instruction 
    std::vector<Instruction> set;

    //string buffer
    std::stringstream ss;
    //section length
    size_t _len = 0;

public:
    InstructionSet() :ss(std::ios::in | std::ios::out | std::ios::binary) {}
    
    void insert(const Instruction& ins){

        //add the instruction to the list
        set.push_back(ins);

        //write the encode to buffer
        uint32_t code = ins.encode();
        ss.write((char*)&code,sizeof(uint32_t));
        _len += sizeof(uint32_t);
    }

    //return the machine code memory space address
    uint32_t content(char* buf){
        uint32_t tmp = _len;
        ss.read(buf, _len);
        _len = 0;
        return tmp;
    }
    
};