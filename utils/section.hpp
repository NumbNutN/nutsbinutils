#pragma once

#include "binbuf.hpp"
#include "directive.hpp"

#include <elf.h>
#include <iostream>
#include <streambuf>


class section{

private:
    std::string _name;
    Elf32_Shdr _sechdr;  /* section header */
    binbuf _buf;

protected:
    
    section(const std::string& name,Elf32_Word type,Elf32_Word addr,Elf32_Word flags = 0) :
            _name(name),
            _sechdr({
                .sh_type = type,
                .sh_flags = flags,
                .sh_addr = addr,
                .sh_size = 0
                }){}

    binbuf& buffer(){
        
        std::ostream out(&_buf);
        //buffer must be flush before return
        //else it will get problem when copy
        out.flush();
        return _buf;
    }

public:

    /*
     * create a section object using section header,basically use when reading from file
     * name is required when construct from a relocatable file
    */
    section(const std::string& name,const Elf32_Shdr& sechdr): _name(name),_sechdr(sechdr){}

    const Elf32_Shdr& getHeader() const{
        return _sechdr;
    }

    const std::string& getName() const{
        return _name;
    }

    uint32_t size() const{
        return _sechdr.sh_size;
    }

    /*
     * the derived have the voluntary to refresh size
    */
    uint32_t& size(){
        return _sechdr.sh_size;
    }

    void setNameIdx(uint32_t idx){
        _sechdr.sh_name = idx;
    }

    void setOffset(uint32_t off){
        _sechdr.sh_offset = off;
    }

    uint32_t flags() const{
        return _sechdr.sh_flags;
    }

    friend std::ostream& operator<<(std::ostream& out,section& sec);
    friend std::istream& operator>>(std::istream& in,section& sec);

    template <typename T>
    friend section& operator<<(section& sec,T dat);

};

// now section operator<< has no right to set put area offset
inline std::ostream& operator<<(std::ostream& out,section& sec)
{
    std::istream in(&sec._buf);
    char tmp[sec.size()];
    in.read(tmp,sec.size());
    out.write(tmp, sec.size());
    out.flush();
    return out;
}

/**
 * the input stream is usually a binary stream from file
*/
inline std::istream& operator>>(std::istream& in,section& sec){

    std::ostream out(&sec._buf);
    char tmp[sec._sechdr.sh_size];
    in.read(tmp, sec._sechdr.sh_size);
    out.write(tmp, sec._sechdr.sh_size);

    return in;
}

template <typename T>
inline section& operator<<(section& sec,T dat){
    std::ostream out(&sec._buf);
    out.write((const char*)&dat,sizeof(T));
    return sec;
}

template<>
inline section& operator<<(section& sec,std::string dat){
    std::ostream out(&sec._buf);
    out << dat;
    char c = '\0';
    out.write(&c,sizeof(char));
}

template<directive_type type>
inline section& operator<<(section& sec,directive<type> directive){
    std::ostream out(&sec._buf);
    out << directive;
}