#pragma once

#include "binbuf.hpp"
#include "sequence.hpp"

#include "utils.h"

#include <string>
#include <iostream>

enum directive_type{
    STRING,
    WORD,
    ZERO,
    ALIGN
};

template <directive_type type>
class Directive :public Sequence{


public:

    Directive(std::string str);
    Directive(uint32_t dat);
    Directive(uint32_t num1,uint32_t num2);
};

template <>
inline Directive<STRING>::Directive(std::string str){
    *this << str;
}

template <>
inline Directive<WORD>::Directive(uint32_t dat){
    *this << dat;
}

template <>
inline Directive<ZERO>::Directive(uint32_t num){
    _size = num*sizeof(char);
    char tmp[_size];
    memset(tmp,0,_size);
    *this << tmp;
}

//this's not a directive that focus on the memory organization with a partial view
//so a current position from outside should be given
template <>
inline Directive<ALIGN>::Directive(uint32_t align,uint32_t pos){
    //check the current pos
    uint32_t nextPos = MOD(pos,align)?(ROUND(pos,align)+(1<<align)):pos;
    _size = nextPos - pos;
    char tmp[_size];
    memset(tmp,0,_size);
    *this << tmp;
}