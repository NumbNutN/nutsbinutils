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
    uint32_t field;
    //TODO read will cause error data in buffer
    //stream.read((char*)&field, sizeof(uint32_t));
    field = -8 + refer_pos - pos();
    std::cout << buffer();
    stream.seekp(0);
    stream.write((char*)&field, sizeof(uint32_t));
    std::cout << buffer();
    refresh();
}