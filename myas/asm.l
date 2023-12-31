%{
#include "mnemonic.hpp"
#include "operand.hpp"
#include "instruction.hpp"
#include "customizable_section.hpp"
#include "parser.h"

#include <string.h>

extern YYSTYPE yylval;

extern "C"{
    extern int yylex (void);
}

%}

%option outfile="scanner.c" header="scanner.h"


ADD "add"|"ADD"
SUB "sub"|"SUB"
MUL "mul"|"MUL"
LDR "ldr"|"LDR"
STR "str"|"STR"
UMULL "umull"|"UMULL" //64位无符号乘法
UMLAL "umlal"|"UMLAL" //64位无符号乘加
MOV "mov"|"MOV"
SVC "svc"|"SVC"

GT  gt|GT
LT  lt|LT
GE  "ge"|"GE"
LE  "le"|"LE"
EQ  "eq"|"EQ"
NE  "ne"|"NE"

Byte "B"|"b"
Word "W"|"w"

SetCondition "S"|"s"

IMMEDIATE "-"?[0-9]+

STRING_LITERAL \"[a-zA-Z_ ]+\"
SYMBOL  [a-zA-Z0-9_]+

R0  [rR]0
R1  [rR]1
R2  [rR]2
R3  [rR]3
R4  [rR]4
R5  [rR]5
R6  [rR]6
R7  [rR]7
R8  [rR]8
R9  [rR]9
R10 [rR]10
R11 [rR]11

%%

{LE}    {yylval.affix = Mnemonic::COND_LE;return FIELD_COND;}
{GT}    {yylval.affix = Mnemonic::COND_GT;return FIELD_COND;}
{LT}    {yylval.affix = Mnemonic::COND_LT;return FIELD_COND;}
{GE}    {yylval.affix = Mnemonic::COND_GE;return FIELD_COND;}
{EQ}    {yylval.affix = Mnemonic::COND_EQ;return FIELD_COND;}
{NE}    {yylval.affix = Mnemonic::COND_NE;return FIELD_COND;}

{SetCondition}  {return FIELD_SETCOND;}

{ADD}   {yylval.opCode = Mnemonic::ADD;return FIELD_OPCODE;}
{SUB}   {yylval.opCode = Mnemonic::SUB;return FIELD_OPCODE;}
{MUL}   {yylval.opCode = Mnemonic::MUL;return FIELD_OPCODE;}
{LDR}   {yylval.opCode = Mnemonic::LDR;return FIELD_OPCODE;}
{STR}   {yylval.opCode = Mnemonic::STR;return FIELD_OPCODE;}
{MOV}   {yylval.opCode = Mnemonic::MOV;return FIELD_OPCODE;}
{SVC}   {yylval.opCode = Mnemonic::SVC;return FIELD_OPCODE;}

{R0}    {yylval.reg = R0;return FIELD_REGISTER;}
{R1}    {yylval.reg = R1;return FIELD_REGISTER;}
{R2}    {yylval.reg = R2;return FIELD_REGISTER;}
{R3}    {yylval.reg = R3;return FIELD_REGISTER;}
{R4}    {yylval.reg = R4;return FIELD_REGISTER;}
{R5}    {yylval.reg = R5;return FIELD_REGISTER;}
{R6}    {yylval.reg = R6;return FIELD_REGISTER;}
{R7}    {yylval.reg = R7;return FIELD_REGISTER;}
{R8}    {yylval.reg = R8;return FIELD_REGISTER;}
{R9}    {yylval.reg = R9;return FIELD_REGISTER;}
{R10}    {yylval.reg = R10;return FIELD_REGISTER;}
{R11}    {yylval.reg = R11;return FIELD_REGISTER;}


{Byte}  {yylval.byteWord = Mnemonic::BYTE;return FIELD_WORDLENGTH;}
{Word}  {yylval.byteWord = Mnemonic::WORD;return FIELD_WORDLENGTH;}

{IMMEDIATE} {yylval.immd = atoi(yytext); return IMMEDIATE;}

".section"  {return DIRECTIVE_SECTION_NAME;}
".word"     {return DIRECTIVE_WORD_NAME;}
".string"   {return DIRECTIVE_STRING_NAME;}
".zero"     {return DIRECTIVE_ZERO_NAME;}
".align"    {return DIRECTIVE_ALIGN_NAME;}
".global"   {return DIRECTIVE_GLOBAL_NAME;}

{STRING_LITERAL}    {

    size_t len = strlen(yytext);
    yytext[len - 1] = '\0';
    yytext[0] = '\0';
    strcpy(yylval.string_literal,yytext+1);
    return STRING_LITERAL;
}

{SYMBOL}        {
    strcpy(yylval.string_literal,yytext);
    //printf("symbol recognize:%d\n",yylineno);
    return SYMBOL_NAME;
}

{}

[!#\-,\[\]\:=]    {return *yytext;}
[\n \t]     {}

%%

