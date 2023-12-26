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

        for(BindingItem& seq:binding_table){
            // refill new address
            if(seq.bind_type == R_ARM_ABS32){}
        }
    }
public:
    struct BindingItem{
        Sequence seq;
        uint32_t bind_type;
    };
public:
    std::string _name;
    SymbolBindingType _type;
    //Record offsets that require repositioning
    std::vector<BindingItem>binding_table;
    Symbol(const std::string& name,SymbolBindingType type = LOCAL):_name(name),_type(type){
        set_base(0);
    }

    void bind(const Sequence& seq,uint32_t bind_type){
        binding_table.push_back({seq,bind_type});
    }
    
};