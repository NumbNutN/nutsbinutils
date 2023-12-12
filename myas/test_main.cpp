#include "mnemonic.hpp"
#include "operand.hpp"
#include "instruction.hpp"

#include "scanner.h"
#include "parser.h"

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
    {LDR,0b01000001 << 20},
    {STR,0b01000000 << 20}

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

int main(int argc,char** argv)
{
    FILE* fp = fopen(argv[1],"r");
    yyrestart(fp);

    // char* buffer = (char*)malloc(1024);
    // FILE* fp = fopen(argv[1],"r");
    // size_t bytes_read = fread(buffer,1,1024,fp);
    // //创建缓存，并将字符串扫描进缓存中
    // YY_BUFFER_STATE bp = yy_scan_string(buffer);
    // //将输入源转为指定内存
	// yy_switch_to_buffer(bp);

    yyparse();

	// //清理内存
	// yy_delete_buffer(bp);
	// yylex_destroy();
	return 0;
}