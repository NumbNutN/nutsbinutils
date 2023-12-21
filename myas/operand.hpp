#pragma once

#include <stdint.h>

#include "tool.hpp"

#include <iostream>

using namespace std;

enum RegType{
    /* Destination register in DataProcess*/
    /* Source/Destination register in SingleDataTransfer */
    Rd = 12,
    /* 1st operand regitser in DataProcess*/
    Rn = 16,
    /* Ra register in MUL */
    Ra,
    /* Immed + rotate / Rm + Shift in Dataprocess */
    Op2 = 0,
    /* ImmedOffset / Rm + Shift in SingleDataTransfer */
    Off = 1
};

enum Register{
    INVALID_REG = 0,
    R0 = 0,R1,R2,R3,R4,R5,R6,R7,R8,      //通用寄存器
    R9,SB=R9,R10,SL=R10,R11,FP=R11,R12,IP=R12,      //分组寄存器
    R13,                             //堆栈指针，最多允许六个不同的堆栈空间           
    SP = R13,                
    R14,                              //链接寄存器，子程序调用保存返回地址
    LR = R14,
    R15,                              //(R15)
    PC = R15,
    CPSR,
    SPSR
};

template<RegType regType>
class Operand{

public:
    enum UpDown{
        DOWN = 0,
        UP = 1
    };
private:

    enum ImmedOff{
        Immediate_Rotation = 1,
        DataProcess_Rm_Shift = 0,
        Immediate_Offset = 0,
        SingleDataTransfer_Rm_Shift = 1
    };

    const uint8_t immedIdx = 0;
    const uint8_t shiftIdx = 4;
    const uint8_t offsetIdx = 0;
    const uint8_t rotateIdx = 8;
    const uint8_t immedOffIdx = 25;
    const uint8_t upDownBitIdx = 23;

    RegType type = regType;
    ImmedOff immedOff = DataProcess_Rm_Shift;
    /* Rd Rn Op2 all feature */
    /* Rd Rn Rm */
    enum Register reg;
    unsigned shift = 0;

    /*Dataprocess op2 only */
    int32_t immed = 0;
    unsigned rotate = 0;

    /*Single data transfer only */
    int32_t offset = 0;
    UpDown upDownBit:1 = UP;                /* 0 down 1 up idx 23 */

    uint32_t code = 0;

    void convert_immediate(int num)
    {
        for(int i=0;i<=30;i+=2)
        {
            int res = ((num << i) | (num >>(32-i)));
            if((res& ~0xFF) == 0)
            {
                immed = res;
                rotate = i / 2;
                return;
            }
        }
    }

public:
    /* 操作数类型 寄存器编号 立即数 偏移值生效先或后 偏移值+或- */

    //Constructor for Rd Rn Rs
    Operand(enum Register reg=INVALID_REG):reg(reg){}

    //Constructor for operand2 & offset
    Operand(int32_t immed);
    Operand(Register reg,unsigned shift);

    //Constructor for offset
    Operand(Register reg,unsigned shift,UpDown upDown);

    friend ostream& operator<<(ostream& out,const Operand& op);

    uint32_t encode(){
        /* type == Rn or Rd or Rm */
        code |= (reg << type);     

        /* type == op2 or offset && immed offset */
        code |= (immedOff << immedOffIdx);  

        /* type == op2 -> immed+rotate */
        code |= (immed << immedIdx);
        code |= (rotate << rotateIdx);

        /* type == offset -> immed */
        code |= (offset << offsetIdx);

        /* type == offset */
        code |= (upDownBit << upDownBitIdx);

        /* type == op2/offset -> rm+shift */
        code |= (shift << shiftIdx);

        return code;
    }

};

/* 偏特化 for Operand2 immed+rotate */
template<>
inline Operand<Op2>::Operand(int32_t immed) {
    convert_immediate(immed);
    immedOff = Immediate_Rotation;
}

/* 偏特化 for Operand2 Reg+shift */
template<>
inline Operand<Op2>::Operand(Register reg,unsigned shift):reg(reg),shift(shift){
    immedOff = DataProcess_Rm_Shift;
}

/* Offset immed */
template<>
inline Operand<Off>::Operand(int32_t offset) :offset(abs(offset)){
    if(offset>0)upDownBit = UP;
    else upDownBit = DOWN;
    immedOff = Immediate_Offset;
}

/* Offset Reg+shift */
template<>
inline Operand<Off>::Operand(Register reg,unsigned shift,UpDown upDown) :reg(reg),shift(shift),upDownBit(upDown){
    immedOff = SingleDataTransfer_Rm_Shift;
}

template<RegType regType>
inline ostream& operator<<(ostream& out,const Operand<regType>& op) {
    out << "R" << op.reg;
    return out;
}

extern Operand<Off> none_offset;
extern Operand<Rn> none_rn;