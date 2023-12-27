#pragma once

#include <elf.h>

#include <vector>
#include <unordered_map>

#include "elf.hpp"
#include "binbuf.hpp"
#include "section.hpp"
#include "symbol_table.hpp"
#include "container.hpp"

#include "utils.h"

/**
 * segment manager
 */
class Segment :public Container<SEGMENT_ALIGN>{

private:
  binbuf _buf;
  Elf32_Phdr _phdr; /* segement header */

protected:


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

    Elf32_Phdr getHeader() const{
        return _phdr;
    }

    friend Segment& operator<<(Segment& ctn,Sequence& seq);
    friend std::ofstream& operator<<(std::ofstream& out,Segment& seg);
};

//segment operator<< has no right to change the get area pointer
inline std::ofstream& operator<<(std::ofstream& out,Segment& seg)
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

inline Segment& operator<<(Segment& ctn,Sequence& seq){
    (Container<3>&) ctn << seq;
    ctn._phdr.p_filesz = ctn.size();
    ctn._phdr.p_memsz = ctn.size();
    return ctn;
}