#pragma once

#include "binbuf.hpp"

#include <elf.h>
#include <iostream>

class section{

private:
    binbuf _buf;
    std::string _name;
    Elf32_Shdr _sechdr;  /* section header */
    std::ostream _out;

protected:
    
    section(const std::string& name,Elf32_Word type,Elf32_Word addr,Elf32_Word flags = 0) :
            _name(name),
            _sechdr({
                .sh_type = type,
                .sh_flags = flags,
                .sh_addr = addr,
                .sh_size = 0
                }),
                _out(std::ostream(&_buf)){}

    std::ostream& binstream(){
        return _out;
    }

    uint32_t& size(){
        return _sechdr.sh_size;
    }

    const binbuf& buffer(){
        
        std::ostream out(&_buf);
        //buffer must be flush before return
        //else it will get problem when copy
        out.flush();
        return _buf;
    }

public:

    const Elf32_Shdr& getSectionHeader() const{
        return _sechdr;
    }

    const std::string& getName() const{
        return _name;
    }

    void setNameIdx(uint32_t idx){
        _sechdr.sh_name = idx;
    }

    void setOffset(uint32_t off){
        _sechdr.sh_offset = off;
    }

    friend std::ostream& operator<<(std::ostream& out,section& sec);

};

inline std::ostream& operator<<(std::ostream& out,section& sec)
{
    char c;
    out.seekp(sec._sechdr.sh_offset, std::ios::beg);
    std::istream in((std::streambuf*)&sec._buf);
    while(in.get(c)){
        out.write(&c, 1);
    }
    return out;
}