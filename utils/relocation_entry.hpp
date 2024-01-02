#pragma once

#include <stdint.h>
#include <elf.h>
#include <iostream>

#include "sequence.hpp"

template <uint32_t type>
class Rel : public Sequence{

public:
    void relocate(uint32_t);
    void relocate(int32_t);

    Rel() = default;

    Rel(const Rel& rel) = delete;
};

template<>
inline void Rel<R_ARM_ABS32>::relocate(uint32_t pos){
    std::iostream stream(&buffer());
    uint32_t field;
    stream.read((char*)&field, sizeof(uint32_t));
    field += pos;
    stream.seekp(0);
    stream.write((char*)&field, sizeof(uint32_t));
    refresh();
}

template<>
inline void Rel<R_ARM_REL32>::relocate(int32_t refer_pos){
    std::iostream stream(&buffer());
    uint16_t field = 0;
    //TODO read will cause error data in buffer

    //as we could only write 2 bytes but not 12 bits
    //we should get 2 bytes
    stream.seekg(0);
    stream.read((char*)&field, sizeof(uint16_t));
    field &= ~0xFFF;
    field |= ((-8 + refer_pos - pos()) & 0xFFF);
    stream.seekp(0);
    stream.write((char*)&field, sizeof(uint16_t));
    refresh();
}