#pragma once

#include <iostream>
#include<unordered_map>

#include "assert.h"

using namespace std;

/**
 * @brief 文法意义上的指令助记符
*/
class Mnemonic{

public:
    enum _Opcode{
        INVALID_OPCODE = 0,  /* 非法命令域 */
        ADD,     //加法指令
        SUB,     //减法指令
        RSB,     //32位逆向减法
        MUL,     //32位乘法指令
        MLA,     //32位乘加指令

        TST,

        MOV,
        MVN,
        CMP,
        CMN,

        MOVW,
        MOVT,

        AND,
        ORR,

        LDR,
        STR,

        UMULL,   //64位无符号乘法
        UMLAL,   //64位无符号乘加

        SVC
    };

    enum asm_affix{
        COND_ALWAYS,
        COND_GT,
        COND_LT,
        COND_LE,
        COND_GE,
        COND_EQ,
        COND_NE,
        ASM_AFFIX_END
    };

    enum asm_byteword{
        WORD = 0,
        BYTE
    };

    

private:

    enum _Opcode opcode; /* 操作码 */
    enum asm_affix cond; /* 条件码 */

    unsigned setCond:1;   /* 是否设置状态位*/   /* 0 unset 1 set  20idx*/
    asm_byteword byteWordBit:1;  /* 0 word 1 byte    22idx*/

    static unordered_map<Mnemonic::_Opcode,uint32_t> op2Code;
    static unordered_map<Mnemonic::asm_affix,uint32_t> cond2Code;

public:

    // Mnemonic(enum _Opcode opcode=INVALID_OPCODE,enum asm_affix cond=COND_ALWAYS,asm_byteword byteWordBit=WORD,bool setCond=false) noexcept : \
    //     opcode(opcode),cond(cond),setCond(setCond),byteWordBit(byteWordBit){}

    Mnemonic(_Opcode opcode,asm_affix cond,asm_byteword byteWordBit,bool setCond) noexcept : \
        opcode(opcode),cond(cond),setCond(setCond),byteWordBit(byteWordBit){}

    //Opcode only  | Opcode with setCond
    Mnemonic(enum _Opcode opcode,bool setCond=false) : Mnemonic(opcode,COND_ALWAYS,WORD,setCond){}

    //Opcode Cond | Opcode Cond setCond
    Mnemonic(enum _Opcode opcode,enum asm_affix cond,bool setCond=false) : Mnemonic(opcode,cond,WORD,setCond){}

    //Opcode Cond ByteWord
    Mnemonic(enum _Opcode opcode,enum asm_affix cond,asm_byteword byteWordBit) : Mnemonic(opcode,cond,byteWordBit,false){}

    //Opcode byteword
    Mnemonic(enum _Opcode opcode,asm_byteword byteWordBit) : Mnemonic(opcode,COND_ALWAYS,byteWordBit,false){}

    friend ostream& operator<<(ostream& out,const Mnemonic& mnemonic);

    uint32_t encode() const{

        uint32_t code = 0;
        code |= Mnemonic::op2Code.at(opcode);
        //条件码
        code |= Mnemonic::cond2Code.at(cond);   
        
        //other field
        code |= (setCond << setCondBitIdx);
        code |= (byteWordBit << byteWordBitIdx);
        return code;
    }

    const uint8_t setCondBitIdx = 20;
    const uint8_t byteWordBitIdx = 22;
};

inline ostream& operator<<(ostream& out,const Mnemonic& mnemonic){
    return out;
}