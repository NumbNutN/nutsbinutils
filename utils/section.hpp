#pragma once

#include "binbuf.hpp"

#include <elf.h>
#include <iostream>
#include <streambuf>

class section{

private:
    binbuf _buf;
    std::string _name;
    Elf32_Shdr _sechdr;  /* section header */

protected:
    
    section(const std::string& name,Elf32_Word type,Elf32_Word addr,Elf32_Word flags = 0) :
            _name(name),
            _sechdr({
                .sh_type = type,
                .sh_flags = flags,
                .sh_addr = addr,
                .sh_size = 0
                }){}

    const binbuf& buffer(){
        
        std::ostream out(&_buf);
        //buffer must be flush before return
        //else it will get problem when copy
        out.flush();
        return _buf;
    }

public:

    /*
     * create a section object using section header,basically use when reading from file
    */
    section(const Elf32_Shdr& sechdr): _sechdr(sechdr){}

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

    friend std::ostream& operator<<(std::ostream& out,const section& sec);
    friend std::istream& operator>>(std::istream& in,section& sec);

    template <typename T>
    friend section& operator<<(section& sec,T dat);

};

inline std::ostream& operator<<(std::ostream& out,const section& sec)
{
    out.seekp(sec._sechdr.sh_offset, std::ios::beg);
    std::istream in((std::streambuf*)&sec._buf);
    char tmp[sec.size()];
    in.get(tmp,sec.size());
    size_t cnt = in.gcount();
    out.write(tmp, sec.size());
    out.flush();
    return out;
}

/**
 * the input stream is usually a binary stream from file
*/
inline std::istream& operator>>(std::istream& in,section& sec){

    char c;
    //record current offset
    std::streambuf::pos_type off = in.tellg();

    in.seekg(sec._sechdr.sh_offset, std::ios::beg);
    std::ostream out(&sec._buf);

    char tmp[sec._sechdr.sh_size];
    in.get(tmp, sec._sechdr.sh_size);
    out.write(tmp, sec._sechdr.sh_size);

    //resume the current offset
    in.seekg(off);

    return in;
}

template <typename T>
inline section& operator<<(section& sec,T dat){
    std::ostream out(&sec._buf);
    out.write((const char*)&dat,sizeof(T));
}

template<>
inline section& operator<<(section& sec,std::string dat){
    std::ostream out(&sec._buf);
    out << dat;
    char c = '\0';
    out.write(&c,sizeof(char));
}