#pragma once
#include <string>
#include <vector>
#include <elf.h>

#include "sequence.hpp"


enum SymbolBindingType{
    LOCAL,
    GLOBAL
};

class Symbol : public Sequence{

public:
// sequence interface
    virtual void set_base(uint32_t new_base) override{

        for(Sequence& seq:binding_table){
            // refill new address
        }
    }
public:
    std::string _name;
    SymbolBindingType _type;
    //Record offsets that require repositioning
    std::vector<Sequence>binding_table;
    Symbol(const std::string& name,SymbolBindingType type = LOCAL):_name(name),_type(type){
        set_base(0);
    }

    void bind(const Sequence& seq){
        binding_table.push_back(seq);
    }
    
};