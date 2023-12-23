#include "mnemonic.hpp"
#include "operand.hpp"
#include "instruction.hpp"
#include "customizable_section.hpp"
#include "relocatable.hpp"

#include "scanner.h"
#include "parser.h"

#include <elf.h>

#include <fstream>
#include <sstream>

#include "elf.hpp"

const size_t opCodeIdx = 21;
const size_t load_store_bitIdx = 20;
unordered_map<Mnemonic::_Opcode,uint32_t> Mnemonic::op2Code  = {
    //Data Process Instruction     SetType + OpCode
    {AND,0b0000000 << opCodeIdx},
    {SUB,0b0000010 << opCodeIdx},
    {RSB,0b0000011 << opCodeIdx},
    {ADD,0b0000100 << opCodeIdx},
    {TST,0b0001000 << opCodeIdx},
    {CMP,0b0001010 << opCodeIdx},
    {CMN,0b0001011 << opCodeIdx},
    {MOV,0b0001101 << opCodeIdx},
    {MVN,0b0001111 << opCodeIdx},
    //Single Data Transfer
    {LDR,0b01000001 << load_store_bitIdx},
    {STR,0b01000000 << load_store_bitIdx},
    {SVC,0b1111 << 24}

};   

unordered_map<Mnemonic::asm_affix,uint32_t> Mnemonic::cond2Code = {
    {Mnemonic::COND_EQ,0x00000000},
    {Mnemonic::COND_NE,0x10000000},
    {Mnemonic::COND_GE,0xA0000000},
    {Mnemonic::COND_LT,0xB0000000},
    {Mnemonic::COND_GT,0xC0000000},
    {Mnemonic::COND_LE,0xD0000000},
    {Mnemonic::COND_ALWAYS,0xE0000000}
};

//create a elf object
Relocatable reloobj;

//current instruction set address, use in lexical parser
CustomizableSection* curInstructionSet;

//current recorded symbol that in a LDR instruction
char* curSymbol;

int main(int argc,char** argv)
{

    //open the ELF file stream
    std::fstream output;
    output.open(argv[1],ios::out | ios::binary);
    
    FILE* fp = fopen(argv[2],"r");
    yyrestart(fp);

    yyparse();

    //rearange the section 
    reloobj.arange();
    
    //write elf file
    output << reloobj;

	return 0;
}