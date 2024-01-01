#pragma once
#include <string>
#include <vector>
#include <elf.h>
#include <memory>

#include "sequence.hpp"
#include "relocation_entry.hpp"


class Symbol : public Rel<R_ARM_ABS32>{

private:
    void _update(){
        for(std::shared_ptr<Rel<R_ARM_ABS32>>& rel:abs_binding_table){
            // refill new address
            rel->relocate(pos());
        }

        for(Rel<R_ARM_REL32>& rel:rel_binding_table){
            rel.relocate((int32_t)pos());
        }
    }
public:
// sequence interface
    virtual void set_base(uint32_t new_base) override{
        Sequence::set_base(new_base);
        _update();
    }

    virtual void set_offset(int32_t new_offset) override{
        Sequence::set_offset(new_offset);
        _update();
    }

public:
    std::string _name;
    unsigned char _type;
    //Record offsets that require repositioning
    std::vector<std::shared_ptr<Rel<R_ARM_ABS32>>>abs_binding_table;
    std::vector<Rel<R_ARM_REL32>>rel_binding_table;
    Symbol(const std::string& name,unsigned char type = STB_LOCAL):_name(name),_type(type){
        set_base(0);
    }

    void bind(const std::shared_ptr<Rel<R_ARM_ABS32>>& rel){
        abs_binding_table.push_back(rel);
        _update();
    }
    
    void bind(const Rel<R_ARM_REL32>& rel){
        rel_binding_table.push_back(rel);
        _update();
    }
};