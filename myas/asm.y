%require "3.5.1"
%language "c"
%defines "parser.h"
%output "parser.c"

%{
#include "binbuf.hpp"
#include "mnemonic.hpp"
#include "operand.hpp"
#include "instruction.hpp"
#include "customizable_section.hpp"
#include "relocatable.hpp"
#include "symbol.hpp"

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

extern Relocatable reloobj;
extern CustomizableSection* curInstructionSet;
extern char* curSymbol;

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
    char string_literal[64];       /* 字符串字面量 */

    //非终结符
    CustomizableSection* seq; /* 指令集 */

    Instruction<COMPLETE_INS>* ins;   /* 不需要重定位指令 */
    Instruction<INCOMPLETE_INS>* incomplete_ins;  /* 需要重定位指令 */

    Mnemonic* mnemonic;  /* 指令助记符 */
    Operand<Rd>* rd;     /* 操作数 */
    Operand<Rn>* rn;
    Operand<Ra>* ra;
    Operand<Op2>* op2;
    Operand<Off>* off;
    
    Directive<WORD>* directive_word;
    Directive<ZERO>* directive_zero;
    Directive<ALIGN>* directive_align;
    Directive<STRING>* directive_string;

    //symbol
    Symbol* symbol;
    
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

%token <string_literal> SYMBOL_NAME      /* 符号 */

//directives
%token <nullptr> DIRECTIVE_WORD_NAME
%token <nullptr> DIRECTIVE_ZERO_NAME
%token <nullptr> DIRECTIVE_STRING_NAME
%token <nullptr> DIRECTIVE_ALIGN_NAME
%token <nullptr> DIRECTIVE_SECTION_NAME
%token <nullptr> DIRECTIVE_GLOBAL_NAME

%type <directive_word> DIRECTIVE_WORD
%type <directive_zero> DIRECTIVE_ZERO
%type <directive_string> DIRECTIVE_STRING
%type <directive_align> DIRECTIVE_ALIGN
%type <symbol> SYMBOL           /* 符号 */

%type <mnemonic> MNEMONIC           /* 指令助记符 */

%type <rd> RD                   /* Source/Destination register */
%type <rn> RN                   /* Base register */
%type <ra> RA                   /* TODO */
%type <reg> RM                   /* Offset register */
%type <op2> OPERAND2            /* 灵活第二操作数 */
%type <off> OFFSET              /* offset */

%type <ins> INSTRUCTION
%type <incomplete_ins> INCOMPLETE_INSTRUCTION
%type <seq> CUSTOM_SECTION
%type <nullptr> TEXT


/* 文法规则 */
%%

TEXT
    : TEXT CUSTOM_SECTION                           {
        //as a customizable section is over
        //fill the incomplete instruction's offset if the symbol is identified within one section
        $2->incompleteIdentified();
        //instruction set over
        //add the section
        reloobj.insert(*$2);
        std::cout << $2->buffer();
        std::cout << reloobj.sectionUnitList[0].buffer();
    }

    // add a global symbol statement to relocatable
    | TEXT DIRECTIVE_GLOBAL_NAME SYMBOL_NAME                     {
        //create a global symbol
        Symbol sym($3,GLOBAL);
        reloobj.insert(sym);
    }

    |                                               {}

CUSTOM_SECTION
    : CUSTOM_SECTION INSTRUCTION               {
        //counter new instruction, insert
        $1 << *$2;
        $$ = $1;
        // cout << bitset<32>($2->encode()) << endl;
    }

    | CUSTOM_SECTION INCOMPLETE_INSTRUCTION          {
        //if a imcomplete instruction could be filled in a single file
        //it should change to a complete instruction before insert into relocable file
        $1->insert(*$2,curSymbol);
        $$ = $1;
    }

    | CUSTOM_SECTION DIRECTIVE_WORD                  {$1->insert(*$2);$$ = $1;}
    | CUSTOM_SECTION DIRECTIVE_ZERO                  {$1->insert(*$2);$$ = $1;}
    | CUSTOM_SECTION DIRECTIVE_STRING                {$1->insert(*$2);$$ = $1;}
    | CUSTOM_SECTION DIRECTIVE_ALIGN                 {$1->insert(*$2);$$ = $1;}

    | CUSTOM_SECTION SYMBOL                          {$1->insert(*$2);$$ = $1;}

    // a section without explict statement ".section" is not allow
    | DIRECTIVE_SECTION_NAME                          {
        //create a instruction set object
        $$ = new CustomizableSection(reloobj);
        curInstructionSet = $$;
    }

INSTRUCTION
    /* SVC */
    : MNEMONIC  IMMEDIATE                       {
        $$ = new Instruction<COMPLETE_INS>(*$1);
    }

    /* Data Processing */
    | MNEMONIC RD ',' OPERAND2                  {
        $$ = new Instruction<COMPLETE_INS>(*$1,*$2,*$4);
    }
    
    | MNEMONIC RD ',' RN ',' OPERAND2           {
        $$ = new Instruction<COMPLETE_INS>(*$1,*$2,*$4,*$6);
    }

    /* MUL */
    | MNEMONIC RD ',' RN ',' RM ',' RA          {

    }

    /* Single Data Transfer */
    | MNEMONIC RD ',' '[' RN ']'                {
        $$ = new Instruction<COMPLETE_INS>(*$1,*$2,*$5);
    }

    /* Pre ; No WriteBack */
    | MNEMONIC RD ',' '[' RN ',' OFFSET ']'     {
        $$ = new Instruction<COMPLETE_INS>(*$1,*$2,*$5,*$7,Instruction<COMPLETE_INS>::PRE,Instruction<COMPLETE_INS>::NOWRITEBACK);
    }

    /* Pre ; WriteBack */
    | MNEMONIC RD ',' '[' RN ',' OFFSET ']' '!' {
        $$ = new Instruction<COMPLETE_INS>(*$1,*$2,*$5,*$7,Instruction<COMPLETE_INS>::PRE,Instruction<COMPLETE_INS>::WRITEBACK);
    }

    /* Post ; WriteBack */
    | MNEMONIC RD ',' '[' RN ']' ',' OFFSET     {
        $$ = new Instruction<COMPLETE_INS>(*$1,*$2,*$5,*$8,Instruction<COMPLETE_INS>::POST,Instruction<COMPLETE_INS>::WRITEBACK);
    }

INCOMPLETE_INSTRUCTION
    /* LDR REG, =LABEL */
    : MNEMONIC RD ',' '=' SYMBOL_NAME                {
        //record the incomplete instruction need to be identified later
        Operand<Rn> pc(PC);
        $$ = new Instruction<INCOMPLETE_INS>(*$1,*$2,pc,Instruction<INCOMPLETE_INS>::PRE,Instruction<INCOMPLETE_INS>::NOWRITEBACK);
        //record the name dirty
        curSymbol = $5;
    }


MNEMONIC
    : FIELD_OPCODE FIELD_SETCOND FIELD_COND {$$ = new Mnemonic($1,$3,true);std::cout << *(Mnemonic*)$$ << std::endl;}     /* Data Process */
    | FIELD_OPCODE FIELD_WORDLENGTH FIELD_COND {$$ = new Mnemonic($1,$3,$2);std::cout << *(Mnemonic*)$$ << std::endl;} /* Single Data Transer */
    | FIELD_OPCODE {$$ = new Mnemonic($1);std::cout << *(Mnemonic*)$$ << std::endl;}
    | FIELD_OPCODE FIELD_SETCOND {$$ = new Mnemonic($1,true);std::cout << *(Mnemonic*)$$ << std::endl;}
    | FIELD_OPCODE FIELD_COND {$$ = new Mnemonic($1,$2);std::cout << *(Mnemonic*)$$ << std::endl;}
    | FIELD_OPCODE FIELD_WORDLENGTH {$$ = new Mnemonic($1,$2);std::cout << *(Mnemonic*)$$ << std::endl;}

RA
    : FIELD_REGISTER  {$$ = new Operand<Ra>($1);}

RD
    : FIELD_REGISTER  {$$ = new Operand<Rd>($1);}

RN
    : FIELD_REGISTER  {$$ = new Operand<Rn>($1);}

RM 
    : FIELD_REGISTER  {$$ = $1;}

OPERAND2
    : RM    {$$ = new Operand<Op2>($1);}
    | '#' IMMEDIATE {$$ = new Operand<Op2>($2);}

OFFSET
    : RM    {$$ = new Operand<Off>($1);}
    | '-' RM    {$$ = new Operand<Off>($2,0,Operand<Off>::DOWN);}
    | '#' IMMEDIATE {$$ = new Operand<Off>($2);}

DIRECTIVE_WORD
    : DIRECTIVE_WORD_NAME IMMEDIATE   {$$ = new Directive<WORD>($2);}

DIRECTIVE_ZERO
    : DIRECTIVE_ZERO_NAME IMMEDIATE   {$$ = new Directive<ZERO>($2);}

DIRECTIVE_STRING
    : DIRECTIVE_STRING_NAME STRING_LITERAL     {$$ = new Directive<STRING>(std::string($2));}

DIRECTIVE_ALIGN
    : DIRECTIVE_ALIGN_NAME IMMEDIATE          {
        //get the current position
        $$ = new Directive<ALIGN>($2,curInstructionSet->size());
    }

SYMBOL
    : SYMBOL_NAME ':'    {$$ = new Symbol($1,LOCAL);}

%%