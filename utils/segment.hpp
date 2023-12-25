#pragma once

#include <elf.h>

#include <vector>
#include <unordered_map>

#include "elf.hpp"
#include "binbuf.hpp"
#include "section.hpp"
#include "symbol_table.hpp"
#include "container.hpp"

/**
 * segment manager
 */
class Segment :public Container{

private:
  binbuf _buf;
  Elf32_Phdr _phdr; /* segement header */
    uint32_t _poff  =0;
    //section list
    std::vector<Section> sectionUnitList;

    //position within segment and section list
    std::unordered_map<uint32_t,Section> section_set;
protected:

    uint32_t allocoffset(uint32_t size,uint32_t align){
    //first check if _off is align
    _poff = MOD(_poff,align)?(ROUND(_poff,align)+(1<<align)):_poff;

    //align the request mem size
    uint32_t tmp = _poff;
    uint32_t map_sz = MOD(size,align)?(ROUND(size,align)+(1<<align)):size;
    _poff += map_sz;

    //seek the pointer
    std::ostream out(&_buf);
    out.seekp(_poff);

    return tmp;
    }

public:

    void set_offset(int32_t new_offset){
        _phdr.p_offset = new_offset;

        Container::set_offset(new_offset);
    }

public:

  Segment(Elf32_Word type, Elf32_Addr vaddr, Elf32_Addr paddr, Elf32_Word flags, Elf32_Word align)
      : _phdr({.p_type = type,
               .p_vaddr = vaddr,
               .p_paddr = paddr,
               .p_flags = flags, 
               .p_align =  align}) {}

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

    /* insert section into segment for management
     * also copy the content
     * call container::insert
    */
    void insert(Section& sec){
        
        uint32_t pos = allocoffset(sec.size(),3);
        sec.set_offset(pos);

        //add section into container management
        sectionUnitList.push_back(sec);
        Container::insert(sectionUnitList.back());

        *this << sec;
        //refresh file size of segment
        _phdr.p_filesz = pos + sec.size();
        //refresh memory size of segment
        _phdr.p_memsz = pos + sec.size();
    }

    Elf32_Phdr getHeader() const{
        return _phdr;
    }

    friend Segment& operator<<(Segment& seg,Section& sec);

    friend std::ostream& operator<<(std::ostream& out,Segment& seg);
};

inline Segment& operator<<(Segment& seg,Section& sec){
    std::ostream out(&seg._buf);
    out << sec;
    return seg;
}

//segment operator<< has no right to change the get area pointer
inline std::ostream& operator<<(std::ostream& out,Segment& seg)
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