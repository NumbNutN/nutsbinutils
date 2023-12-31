#pragma once

#include <elf.h>

#include <vector>
#include <unordered_map>

#include "section.hpp"
#include "customizable_section.hpp"
#include "container.hpp"

#include "utils.h"

/**
 * segment manager
 */
class Segment :public Container<SEGMENT_ALIGN>{

public:
    Elf32_Word _type;
    Elf32_Addr _addr;
    Elf32_Word _flags;

    std::vector<CustomizableSection> cus_sec_list;
public:

    Segment(Elf32_Word type, Elf32_Addr vaddr, Elf32_Word flags)
      : _type(type),_addr(vaddr),_flags(flags) {}

    // segment(segment& seg) = default;

    void insert(CustomizableSection& sec){
        cus_sec_list.push_back(sec);
        *this << (Sequence&)sec;
    }

    Elf32_Phdr getHeader() const{
        return Elf32_Phdr{
            .p_type = _type,
            .p_offset = pos(),
            //TODO so far we consider the virtual address to be the position in file
            .p_vaddr = pos(),
            .p_paddr = pos(),
            .p_filesz = size(),
            .p_memsz = size(),
            .p_flags = _flags,
            .p_align = 1024,
        };
    }

};