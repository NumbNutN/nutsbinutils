%require "3.5.1"
%language "c"
%defines "parser.h"
%output "parser.c"

%{
#include "mnemonic.hpp"
#include "operand.hpp"
#include "instructionSet.hpp"
#include "instruction.hpp"
#include "elf.hpp"
#include "relocationFile.hpp"
#include "binbuf.hpp"

#include <iostream>
#include <bitset>

#include "scanner.h"

#include <stdarg.h>

extern int yylineno;

void yyerror(char *s, ...) // 变长参数错误处理函数
{
  va_list ap;
  va_start(ap, s);
  fprintf(stderr, "%d:error:", yylineno); // 错误行号
  vfprintf(stderr, s, ap);
  fprintf(stderr, "\n");
}

extern "C" int yylex();

extern relocation_file reloobj;
extern InstructionSet* curInstructionSet;

%}

//yylval
%union{

    //无语义终结符 占位符
    void* nullpointer;
    //终结符
    Mnemonic::asm_affix affix;      /* 条件码枚举值 */
    Mnemonic::_Opcode opCode;  /* 操作码枚举值 */
    Mnemonic::asm_byteword byteWord;      /* 字节/字枚举值 */

    Register reg;     /* 寄存器 */
    uint32_t immd;             /* 立即数常量 */
    char* string_literal;       /* 字符串字面量 */

    //非终结符
    InstructionSet* insSet; /* 指令集 */
    Instruction* ins;   /* 指令 */
    Mnemonic* mnemonic;  /* 指令助记符 */
    Operand<Rd>* rd;     /* 操作数 */
    Operand<Rn>* rn;
    Operand<Ra>* ra;
    Operand<Op2>* op2;
    Operand<Off>* off;
    
    directive<WORD>* dot_word;
    directive<ZERO>* dot_zero;
    directive<ALIGN>* dot_align;
    directive<STRING>* dot_string;
    
}

/* token 源自文件asm_enum.h */

%token <nullptr> ','

%token <affix> FIELD_COND           /* 条件码词缀 */

%token <nullptr> FIELD_SETCOND      /* 设置条件码词缀 */

%token <byteWord>  FIELD_WORDLENGTH   /* 设置操作内存大小 */

%token <opCode> FIELD_OPCODE        /* 操作码 */

%token <immd> IMMEDIATE             /* 立即数 */

%token <reg> FIELD_REGISTER   /* 寄存器 */

%token <string_literal> STRING_LITERAL /* 字符串字面量 */

%token <string_literal> SYMBOL      /* 符号 */

//directives
%token <nullptr> DOT_WORD_NAME
%token <nullptr> DOT_ZERO_NAME
%token <nullptr> DOT_STRING_NAME
%token <nullptr> DOT_ALIGN_NAME
%token <nullptr> DOT_SECTION_NAME

%type <dot_word> DOT_WORD
%type <dot_zero> DOT_ZERO
%type <dot_string> DOT_STRING
%type <dot_align> DOT_ALIGN

%type <mnemonic> MNEMONIC           /* 指令助记符 */

%type <rd> RD                   /* Source/Destination register */
%type <rn> RN                   /* Base register */
%type <ra> RA                   /* TODO */
%type <reg> RM                   /* Offset register */
%type <op2> OPERAND2            /* 灵活第二操作数 */
%type <off> OFFSET              /* offset */

%type <ins> INSTRUCTION
%type <insSet> INSTRUCTION_SET
%type <nullptr> TEXT


/* 文法规则 */
%%

TEXT
    : INSTRUCTION_SET                           {
        //instruction set over
        //add the section
        reloobj.insert(*$1);
    }

INSTRUCTION_SET
    : INSTRUCTION_SET INSTRUCTION               {
        //counter new instruction, insert
        $1->insert(*$2);
        $$ = $1;
        cout << bitset<32>($2->encode()) << endl;
    }

    | INSTRUCTION_SET DOT_WORD                  {$1->insert(*$2);$$ = $1;}
    | INSTRUCTION_SET DOT_ZERO                  {$1->insert(*$2);$$ = $1;}
    | INSTRUCTION_SET DOT_STRING                {$1->insert(*$2);$$ = $1;}
    | INSTRUCTION_SET DOT_ALIGN                 {$1->insert(*$2);$$ = $1;}

    | INSTRUCTION_SET SYMBOL                    {$1->insert(std::string($2));$$ = $1;}

    // a section without explict statement ".section" is not allow
    | DOT_SECTION_NAME                          {
        //create a instruction set object
        $$ = new InstructionSet();
        curInstructionSet = $$;
    }

INSTRUCTION
    /* SVC */
    : MNEMONIC  IMMEDIATE                       {
        $$ = new Instruction(*$1);
    }

    /* Data Processing */
    | MNEMONIC RD ',' OPERAND2                  {
        $$ = new Instruction(*$1,*$2,*$4);
    }
    
    | MNEMONIC RD ',' RN ',' OPERAND2           {
        $$ = new Instruction(*$1,*$2,*$4,*$6);
    }

    /* MUL */
    | MNEMONIC RD ',' RN ',' RM ',' RA          {

    }

    /* Single Data Transfer */
    | MNEMONIC RD ',' '[' RN ']'                {
        $$ = new Instruction(*$1,*$2,*$5);
    }

    /* Pre ; No WriteBack */
    | MNEMONIC RD ',' '[' RN ',' OFFSET ']'     {
        $$ = new Instruction(*$1,*$2,*$5,*$7,Instruction::PRE,Instruction::NOWRITEBACK);
    }

    /* Pre ; WriteBack */
    | MNEMONIC RD ',' '[' RN ',' OFFSET ']' '!' {
        $$ = new Instruction(*$1,*$2,*$5,*$7,Instruction::PRE,Instruction::WRITEBACK);
    }

    /* Post ; WriteBack */
    | MNEMONIC RD ',' '[' RN ']' ',' OFFSET     {
        $$ = new Instruction(*$1,*$2,*$5,*$8,Instruction::POST,Instruction::WRITEBACK);
    }


MNEMONIC
    : FIELD_OPCODE FIELD_SETCOND FIELD_COND {$$ = new Mnemonic($1,$3,true);std::cout << *(Mnemonic*)$$ << std::endl;}     /* Data Process */
    | FIELD_OPCODE FIELD_WORDLENGTH FIELD_COND {$$ = new Mnemonic($1,$3,$2);std::cout << *(Mnemonic*)$$ << std::endl;} /* Single Data Transer */
    | FIELD_OPCODE {$$ = new Mnemonic($1);std::cout << *(Mnemonic*)$$ << std::endl;}
    | FIELD_OPCODE FIELD_SETCOND {$$ = new Mnemonic($1,true);std::cout << *(Mnemonic*)$$ << std::endl;}
    | FIELD_OPCODE FIELD_COND {$$ = new Mnemonic($1,$2);std::cout << *(Mnemonic*)$$ << std::endl;}
    | FIELD_OPCODE FIELD_WORDLENGTH {$$ = new Mnemonic($1,$2);std::cout << *(Mnemonic*)$$ << std::endl;}

RA
    : FIELD_REGISTER  {$$ = new Operand<Ra>($1);std::cout << *$$;}

RD
    : FIELD_REGISTER  {$$ = new Operand<Rd>($1);std::cout << *$$;}

RN
    : FIELD_REGISTER  {$$ = new Operand<Rn>($1);std::cout << *$$;}

RM 
    : FIELD_REGISTER  {$$ = $1;}

OPERAND2
    : RM    {$$ = new Operand<Op2>($1);}
    | '#' IMMEDIATE {$$ = new Operand<Op2>($2);}

OFFSET
    : RM    {$$ = new Operand<Off>($1);}
    | '-' RM    {$$ = new Operand<Off>($2,0,Operand<Off>::DOWN);}
    | '#' IMMEDIATE {$$ = new Operand<Off>($2);}

DOT_WORD
    : DOT_WORD_NAME IMMEDIATE   {$$ = new directive<WORD>($2);}

DOT_ZERO
    : DOT_ZERO_NAME IMMEDIATE   {$$ = new directive<ZERO>($2);}

DOT_STRING
    : DOT_STRING_NAME STRING_LITERAL     {$$ = new directive<STRING>(std::string($2));}

DOT_ALIGN
    : DOT_ALIGN_NAME IMMEDIATE          {
        //get the current position
        $$ = new directive<ALIGN>($2,curInstructionSet->size());
    }

%%