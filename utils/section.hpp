#pragma once

#include "container.hpp"
#include "binbuf.hpp"
#include "directive.hpp"
#include "elf.hpp"

#include <elf.h>
#include <iostream>
#include <streambuf>
#include <fstream>

class Section : public Container<0>{

private:

    bool _isbinding = false;

protected:

    Section(const std::string& name,Elf32_Word type,Elf32_Word addr,Elf32_Word flags = 0) :
            _name(name),
            _type(type),
            _flags(flags),
            _addr(addr)
            {}

    /*
     * create a section object using section header,basically use when reading from file
     * name is required when construct from a relocatable file
    */
    Section(const std::string& name,const Elf32_Shdr& sechdr): 
            _name(name),
            _type(sechdr.sh_type),
            _flags(sechdr.sh_flags),
            _addr(sechdr.sh_addr){}

public:

    std::string _name;
    Elf32_Word _type;
    Elf32_Word _flags;
    Elf32_Word _addr;

    //this is a default constructor for if a std::map object is created to manage the section
    Section(){}

    template<directive_type type>
    friend Section& operator<<(Section& sec,Directive<type>& directive);

    
    friend Section& operator<<(Section& ctn,Sequence& seq);

};

template<directive_type type>
inline Section& operator<<(Section& sec,Directive<type>& directive){
    std::ostream out(&sec._buf);
    out << directive;
    return sec;
}