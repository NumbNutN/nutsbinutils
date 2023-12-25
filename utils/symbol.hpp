#pragma once
#include <string>
#include <elf.h>

#include "sequence.hpp"

enum SymbolBindingType{
    LOCAL,
    GLOBAL
};

class Symbol : public Sequence{

private:

public:
    std::string _name;
    SymbolBindingType _type;
    Symbol(const std::string& name,uint32_t off_in_sec,SymbolBindingType type = LOCAL):_name(name),_type(type){
        set_offset(off_in_sec);
        set_base(0);
    }
};