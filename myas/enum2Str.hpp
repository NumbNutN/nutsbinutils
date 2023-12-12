#include "mnemonic.hpp"

char* instructionMnemonic2Str(Mnemonic::_Opcode opcode);
char* cond2Str(Mnemonic::asm_affix cond);

inline ostream& operator<<(ostream& out,const Mnemonic& mnemonic) {
    out << instructionMnemonic2Str(mnemonic.opcode) << cond2Str(mnemonic.cond);
    return out;
}


inline char* instructionMnemonic2Str(Mnemonic::_Opcode opcode)
{
    switch(opcode)
    {
        case Mnemonic::ADD:
            return "ADD";
        case Mnemonic::SUB:
            return "SUB";
        case Mnemonic::RSB:
            return "RSB";
        case Mnemonic::MUL:
            return "MUL";
        case Mnemonic::MLA:
            return "MLA";
        case Mnemonic::MOV:
            return "MOV";
        case Mnemonic::MOVW:
            return "MOVW";
        case Mnemonic::MOVT:
            return "MOVT";
        case Mnemonic::MVN:
            return "MVN";
        case Mnemonic::CMP:
            return "CMP";
        case Mnemonic::LDR:
            return "LDR";
        case Mnemonic::STR:
            return "STR";
        case Mnemonic::AND:
            return "AND";
        case Mnemonic::ORR:
            return "ORR";
        default:
            assert("Uncognize instruction_mnemonic");
    }
}

inline char* cond2Str(Mnemonic::asm_affix cond)
{
    switch(cond)
    {
        case Mnemonic::COND_NE:
            return "NE";
        case Mnemonic::COND_EQ:
            return "EQ";
        case Mnemonic::COND_GT:
            return "GT";
        case Mnemonic::COND_LT:
            return "LT";
        case Mnemonic::COND_GE:
            return "GE";
        case Mnemonic::COND_LE:
            return "LE";
        case Mnemonic::COND_ALWAYS:
            return "";
    }
}