#pragma once

#include "mnemonic.hpp"
#include "operand.hpp"

#include <initializer_list>

class Instruction{

public:
    enum PrePostIndex{
        NOPREPOST = 0,
        POST = 0,
        PRE = 1
    };

    enum WriteBack{
        NOWRITEBACK = 0,
        WRITEBACK = 1
    };
private:
    // unsigned cond:4;
    // unsigned op2SetImmed:1;
    // unsigned opCode:4;
    // unsigned setCond:1;
    // unsigned rn:4;
    // unsigned rd:4;
    // unsigned op2:12;


    /* Single Data Transer only */
    PrePostIndex prePostIndexingBit:1;   /* 0 post 1 pre idx 24 */
    WriteBack writeBack:1;               /* 0 no 1 yes idx 21 */

    const uint8_t prePostIndexingBitIdx = 24; 
    const uint8_t writeBackBitIdx = 21;

    //code
    uint32_t code;
public:

    /**
     * SVC index
    */
    Instruction(const Mnemonic& mnemonic){
        code |= mnemonic.encode();
    }

    /**
     * Dataprocess rd rn op2
    */
    Instruction(const Mnemonic& mnemonic,Operand<Rd>& rd,Operand<Rn>& rn,Operand<Op2>& op2){
        code |= mnemonic.encode();
        code |= rd.encode();
        code |= rn.encode();
        code |= op2.encode();
    }
    /**
     * Mov rd op2
    */
    Instruction(const Mnemonic& mnemonic,Operand<Rd>& rd,Operand<Op2>& op2):
        Instruction(mnemonic,rd,none_rn,op2) {}

    /*
     * SingleDataTransfer rd , [rn,off]
    */
    Instruction(const Mnemonic& mnemonic,Operand<Rd>& rd,Operand<Rn>& rn,Operand<Off>& off,PrePostIndex prePostIndexingBit = NOPREPOST, WriteBack writeBack = NOWRITEBACK){
        code |= mnemonic.encode();
        code |= rd.encode();
        code |= rn.encode();
        code |= off.encode();
        code |= (prePostIndexingBit << prePostIndexingBitIdx);
        code |= (writeBack << writeBackBitIdx);
    }

    /*
     * SingleDataTransfer rd , [rn]
    */
    Instruction(const Mnemonic& mnemonic,Operand<Rd>& rd,Operand<Rn>& rn):
        Instruction(mnemonic,rd,rn,none_offset){}


    uint32_t encode() const {
        return code;
    }
    /**
     * MUL Rd Rn Rs Rm
    */
};