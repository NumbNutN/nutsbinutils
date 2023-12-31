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

    Elf32_Shdr _sechdr;  /* section header */
    bool _isbinding = false;

protected:
    std::string _name;
    Section(const std::string& name,Elf32_Word type,Elf32_Word addr,Elf32_Word flags = 0) :
            _name(name),
            _sechdr({
                .sh_type = type,
                .sh_flags = flags,
                .sh_addr = addr,
                .sh_size = 0
                }){}

    /*
     * create a section object using section header,basically use when reading from file
     * name is required when construct from a relocatable file
    */
    Section(const std::string& name,const Elf32_Shdr& sechdr): _name(name),_sechdr(sechdr){}

public:

    //this is a default constructor for if a std::map object is created to manage the section
    Section(){}

    const Elf32_Shdr& getHeader() const{
        return _sechdr;
    }

    void setHeader(const Elf32_Shdr& shdr){
        _sechdr = shdr;
    }

    const std::string& getName() const{
        return _name;
    }

    void setNameIdx(uint32_t idx){
        _sechdr.sh_name = idx;
    }

    uint32_t getNameIdx(){
        return _sechdr.sh_name;
    }

    uint32_t getOffset()const{
        return _sechdr.sh_offset;
    }

    void setAddr(uint32_t addr){
        _sechdr.sh_addr = addr;
    }

    uint32_t flags() const{
        return _sechdr.sh_flags;
    }

    template<directive_type type>
    friend Section& operator<<(Section& sec,Directive<type>& directive);

    
    friend Section& operator<<(Section& ctn,Sequence& seq);

};

inline Section& operator<<(Section& ctn,Sequence& seq){
    (Container<0>&)ctn << seq;
    ctn._sechdr.sh_size = ctn.size();
    return ctn;
}

template<directive_type type>
inline Section& operator<<(Section& sec,Directive<type>& directive){
    std::ostream out(&sec._buf);
    out << directive;
    return sec;
}