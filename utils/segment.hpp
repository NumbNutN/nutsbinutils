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

    // segment(segment& seg) = default;

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

    /* insert section into segment for management
     * also copy the content
    */
    void insert(section& sec){

        sectionUnitList.push_back(sec);
        *this << sec;
        //refresh file size of segment
        _phdr.p_filesz += sec.size();
        //refresh memory size of segment
        _phdr.p_memsz += sec.size();
    }

    void link(){

        //linking the section with similar flags together
        //std::ostream out(&_buf);
        for(section& sec:sectionUnitList){
            //out << sec;


        }
    }

    Elf32_Phdr getHeader() const{
        return _phdr;
    }

    friend segment& operator<<(segment& seg,section& sec);

    friend std::ostream& operator<<(std::ostream& out,segment& seg);
};

inline segment& operator<<(segment& seg,section& sec){
    std::ostream out(&seg._buf);
    out << sec;
    return seg;
}

//segment operator<< has no right to change the get area pointer
inline std::ostream& operator<<(std::ostream& out,segment& seg)
{
    std::istream in(&seg._buf);
    //write as segment size says
    char tmp[seg.getHeader().p_filesz];
    in.read(tmp, seg.getHeader().p_filesz);
    out.write(tmp, seg.getHeader().p_filesz);
    //flush the content so we see the content at once it put into the output stream
    out.flush();

    return out;
}