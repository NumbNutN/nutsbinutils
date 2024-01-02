#pragma once

#include <vector>
#include <sstream>
#include <iostream>
#include <memory>


#include "binbuf.hpp"
#include "instruction.hpp"
#include "directive.hpp"
#include "section.hpp"
#include "symbol.hpp"

#include "utils.h"

class CustomizableSection : public Section{
    
protected:

    //record <position,symbol used instruction>
    struct reloIns_type{
        std::string sym;
        std::shared_ptr<Instruction<INCOMPLETE_INS>> ins;
    };

    std::vector<reloIns_type> reloInsSet;

public:
    //local view symbol set
    std::vector<std::shared_ptr<Symbol>> symbol_set;

    //literal symbol set
    std::vector<std::shared_ptr<Symbol>> literal_symbol_set;

    //relo entry list
    std::vector<std::shared_ptr<Rel<R_ARM_ABS32>>>relo_abs_list;
    std::vector<std::shared_ptr<Rel<R_ARM_REL32>>>relo_rel_list;

public:
// container interface
    void set_base(uint32_t new_base){
        
        //set self base
        Section::set_base(new_base);   

        //rebase for global symbol included in the section
        for(std::shared_ptr<Symbol>& sym: symbol_set){
            sym->set_base(pos());
        }
    }

    void set_offset(int32_t new_offset){

        Section::set_offset(new_offset);

        //rebase for global symbol included in the section
        for(std::shared_ptr<Symbol>& sym: symbol_set){
            sym->set_base(pos());
        }
    }

public:

    CustomizableSection(const std::string& name = ".text") : 
        Section(
                name,
                SHT_PROGBITS,
                0x0,
                SHF_ALLOC | SHF_EXECINSTR){}

/**
 * insert method group
 * call when analyse the assemble file
*/

    void insert(Instruction<INCOMPLETE_INS>* ins,const std::string sym){

        //add to the incomplete set
        ins->set_offset(size());
        reloInsSet.push_back({sym,std::shared_ptr<Instruction<INCOMPLETE_INS>>(ins)});
        //write the encode to buffer
        (Container<0>&)*this << (Sequence&)*ins;
        std::cout << buffer();
    }

    void insert(Instruction<COMPLETE_INS>& ins){

        //add to the incomplete set
        ins.set_offset(size());
        //write the encode to buffer
        (Container<0>&)*this << (Sequence&)ins;
        std::cout << buffer();
    }

    template <directive_type type>
    void insert(Directive<type>& obj){
        (Container<0>&)*this << (Sequence&)obj;
    }

    //insert a symbol
    void insert(Symbol* sym){
        symbol_set.push_back(std::shared_ptr<Symbol>(sym));
        (Container<0>&)*this << (Sequence&)*sym;
    }

    //insert a absolute address to literal pool
    //it is a symbol  with R_ARM_ABS32

    //insert into literal pool is a insert later method
    //it is certainly fixed when all the insert done
    void insertLiteral(const std::shared_ptr<Rel<R_ARM_REL32>>& ins,Symbol& sym){
        
        std::shared_ptr<Symbol> literal = std::shared_ptr<Symbol>(new Symbol("$"+sym._name,STB_LOCAL));
        
        //ins insert to relo table
        relo_rel_list.push_back(ins);

        // literal binding the instruction
        literal->bind(ins);
        literal->set_offset(size());

        // literal insert to symbol table
        literal_symbol_set.push_back(literal);

        // literal insert to relo table
        relo_abs_list.push_back(std::dynamic_pointer_cast<Rel<R_ARM_ABS32>>(literal));
        
        // symbol binding the literal
        sym.bind(relo_abs_list.back());

    }

public:
    //get symbol by relocatable
    bool symbolExist(const std::string& str){
        for(std::shared_ptr<Symbol>& sym:symbol_set){
            if(sym->_name == str){
                return true;
            }
        }
        return false;
    }

    //get symbol
    Symbol& getSymbol(const std::string& str){
        for(std::shared_ptr<Symbol>& sym:symbol_set){
            if(sym->_name == str){
                return *sym;
            }
        }
    }

    void refreshAll(){
        //now write all the relocatable entry
        for(auto relo:relo_abs_list){
            this->refresh(*relo);
        }
        for(auto relo:relo_rel_list){
            this->refresh(*relo);
        }
    }

    //if symbol is defined and used within one section
    //it can be identified
    void incompleteIdentified(){

        //for each incomplete instruction, find if symbol is defined within one section
        for(std::vector<reloIns_type>::iterator it = reloInsSet.begin();it!=reloInsSet.end();){
            //get the field in instruction need to be addressed
            
            const reloIns_type& unit = *it;
            const std::string& name = unit.sym;

            //get the incomlete instruction
            std::shared_ptr<Instruction<INCOMPLETE_INS>> incomplete_ins = unit.ins;

            //see if the symbol is existed in the same section
            for(std::shared_ptr<Symbol>& symbol: symbol_set){
                if(symbol->_name == name){

                    //create a symbol in literal pool
                    insertLiteral(incomplete_ins,*symbol);

                    it = reloInsSet.erase(it);
                    // finish the relo instruction
                    break;
                }else{
                    it++;
                }
            }
        }

        //merge literal pool and basic symbol table
        symbol_set.insert(symbol_set.end(), literal_symbol_set.begin(), literal_symbol_set.end());

        refreshAll();
    }

    
};