#pragma once

#include <vector>
#include <sstream>
#include <iostream>


#include "binbuf.hpp"
#include "instruction.hpp"
#include "directive.hpp"
#include "section.hpp"
#include "symbol.hpp"

class CustomizableSection : public Section{

private:
    
protected:
    uint32_t curLiteralPoolPos = 0;
    //local view symbol set
    //record <symbol,abosulte position>
    std::vector<Symbol> symbol_set;

    //record <position,symbol used instruction>
    struct reloIns_type{
        Instruction<INCOMPLETE_INS> ins;
        std::string sym;
    };

    std::vector<reloIns_type> reloInsSet;

public:
    //global symbol definition
    //they should be reset offset if section is rebased
    //record <symbol,abosulte position>
    std::vector<Symbol> gloSymbolSet;


public:
// container interface
    void set_base(uint32_t new_base){
        
        //set self base
        Section::set_base(new_base);   

        //rebase for global symbol included in the section
        for(Symbol& sym: gloSymbolSet){
            sym.set_base(pos());
        }
    }

    void set_offset(int32_t new_offset){

        Section::set_offset(new_offset);

        //rebase for global symbol included in the section
        for(Symbol& sym: gloSymbolSet){
            sym.set_base(pos());
        }
    }

public:

    CustomizableSection(elf& elfbase) : 
        Section((elf&)elfbase,
                ".text",
                SHT_PROGBITS,
                0x0,
                SHF_ALLOC | SHF_EXECINSTR){}

/**
 * insert method group
 * call when analyse the assemble file
*/
    void insert(const Instruction<COMPLETE_INS>& ins){

        //write the encode to buffer
        uint32_t code = ins.encode();
        *this << code;
        size() += sizeof(uint32_t);
    }

    void insert(Instruction<INCOMPLETE_INS>& ins,const std::string sym){

        //add to the incomplete set
        ins.set_offset(size());
        reloInsSet.push_back({ins,sym});
        //write the encode to buffer
        uint32_t code = ins.encode();
        *this << code;
        size() += sizeof(uint32_t);
    }

    template <directive_type type>
    void insert(Directive<type>& obj){
        *this << obj;
        size() += obj.size();
    }

    // //insert a symbol
    // //a symbol use on space
    // void insert(const std::string& str,enum SymbolBindingType type = LOCAL){
    //     symbol_set.push_back(Symbol(str,size()));
    // }

    //insert a symbol
    void insert(const Symbol& sym){
        symbol_set.push_back(sym);
    }

    // //insert a global symbol
    // //symbol will be automatically set new base
    // void insertGlobal(const std::string& str,uint32_t pos){
    //     Symbol sym(str,pos);
    //     gloSymbolSet.push_back(sym);
    //     Container::insert(gloSymbolSet.back());
    // }

    //insert a absolute address to literal pool
    //it is a symbol  with R_ARM_ABS32

    //insert into literal pool is a insert later method
    //it is certainly fixed when all the insert done
    void insertLiteral(const Instruction<INCOMPLETE_INS>& ins,Symbol& sym){
        
        Symbol literal("$"+sym._name,LOCAL);
        // literal binding the instruction
        literal.bind(ins);

        // symbol binding the literal
        sym.bind(literal);

        literal.set_offset(size());
        insert(sym);

    }

public:
//get symbol by relocatable
    bool symbolExist(const std::string& str){
        for(Symbol& sym:symbol_set){
            if(sym._name == str){
                return true;
            }
        }
        return false;
    }

    //get symbol
    Symbol& getSymbol(const std::string& str){
        for(Symbol& sym:symbol_set){
            if(sym._name == str){
                return sym;
            }
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
            Instruction<INCOMPLETE_INS> incomplete_ins = unit.ins;

            //see if the symbol is existed in the same section
            for(Symbol& symbol: symbol_set){
                if(symbol._name == name){

                    //create a symbol in literal pool
                    insertLiteral(incomplete_ins,symbol);

                    it = reloInsSet.erase(it);
                }else{
                    it++;
                }
            }
        }
    }

    friend std::ofstream& operator>>(std::ifstream& in,CustomizableSection& sec);
    
};

/**
 * 
*/
inline std::ofstream& operator>>(std::ifstream& in,CustomizableSection& sec){
    
    //read section header   and   read the buffer content
    in >> (Section&)sec;

}