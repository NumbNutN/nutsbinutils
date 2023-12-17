#pragma once

#include <elf.h>

#include <vector>

#include "elf.hpp"
#include "binbuf.hpp"
#include "section.hpp"

/**
 * segment manager
 */
class segment {

private:
  binbuf _buf;
  Elf32_Phdr _phdr; /* segement header */

    //section list
    std::vector<section> sectionUnitList;
public:

  segment(Elf32_Word type, Elf32_Addr vaddr, Elf32_Addr paddr, Elf32_Word flags)
      : _phdr({.p_type = type,
               .p_vaddr = vaddr,
               .p_paddr = paddr,
               .p_flags = flags}) {}

    const binbuf &buffer() {
        std::ostream out(&_buf);
        // buffer must be flush before return
        // else it will get problem when copy
        out.flush();
        return _buf;
    }

    uint32_t& size(){
        return _phdr.p_filesz;
    }

    void setOffset(uint32_t off){
        _phdr.p_offset = off;
    }

    void insert(const section& sec){

        sectionUnitList.push_back(sec);
    }

    void link(){

        //linking the section with similar flags together
        std::ostream out(&_buf);
        for(section sec:sectionUnitList){
            out << sec;
            //refresh size of segment
            size() += sec.size();
        }
    }

    Elf32_Phdr getHeader() const{
        return _phdr;
    }

    template <typename T>
    friend segment& operator<<(segment& seg,T dat);

    friend std::ostream& operator<<(std::ostream& out,const segment& seg);
};

template <typename T>
inline segment& operator<<(segment& seg,T dat){
    std::ostream out(&seg._buf);
    out.write((const char*)&dat,sizeof(T));
}

template<>
inline segment& operator<<(segment& seg,const section& sec){
    std::ostream out(&seg._buf);
    out << sec;
}

inline std::ostream& operator<<(std::ostream& out,const segment& seg)
{
    char c;
    out.seekp(seg._phdr.p_offset, std::ios::beg);
    std::istream in((std::streambuf*)&seg._buf);
    while(in.get(c)){
        out.write(&c, 1);
    }
    return out;
}