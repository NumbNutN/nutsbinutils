#pragma once

#include "mnemonic.hpp"
#include "operand.hpp"

#include <initializer_list>

enum instruction_type{
    INCOMPLETE_INS,
    COMPLETE_INS
};

template <instruction_type type>
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

    /* Single Data Transer only */
    PrePostIndex prePostIndexingBit:1;   /* 0 post 1 pre idx 24 */
    WriteBack writeBack:1;               /* 0 no 1 yes idx 21 */

    uint8_t prePostIndexingBitIdx = 24; 
    uint8_t writeBackBitIdx = 21;

    //code
    uint32_t code;
public:

    Instruction(const Instruction& ins) = default;

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
    /* incomplete instruction */
    Instruction(const Mnemonic& mnemonic,Operand<Rd>& rd,Operand<Rn>& rn,PrePostIndex prePostIndexingBit, WriteBack writeBack);

    /*
     * SingleDataTransfer rd , [rn]
    */
    Instruction(const Mnemonic& mnemonic,Operand<Rd>& rd,Operand<Rn>& rn):
        Instruction(mnemonic,rd,rn,none_offset){}

    /*
        * Construct a instruction only incomplete instruction
        * This is only happen when an incomplete instruction turn into a complete once
    */    
    // Instruction(const Instruction<INCOMPLETE_INS>& ins);
    
    /* set the offset section of a instruction 
     * for incomplete instruction only
    */
    void setOff(Operand<Off>&);

    uint32_t encode() const {
        return code;
    }
    /**
     * MUL Rd Rn Rs Rm
    */
};

template<>
inline Instruction<INCOMPLETE_INS>::Instruction(const Mnemonic& mnemonic,Operand<Rd>& rd,Operand<Rn>& rn,PrePostIndex prePostIndexingBit, WriteBack writeBack){
    code |= mnemonic.encode();
    code |= rd.encode();
    code |= rn.encode();
    code |= (prePostIndexingBit << prePostIndexingBitIdx);
    code |= (writeBack << writeBackBitIdx);
}


// template<>
// inline Instruction<COMPLETE_INS>::Instruction(const Instruction<INCOMPLETE_INS>& ins){
//     code = ins.encode();
// }

template<>
inline void Instruction<INCOMPLETE_INS>::setOff(Operand<Off>& off){
    code |= off.encode();
}

// template<instruction_type type>
// inline Instruction<type>::Instruction(const Instruction<type>& ins) = default;