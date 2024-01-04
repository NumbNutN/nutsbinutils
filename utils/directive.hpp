#pragma once

#include "binbuf.hpp"
#include "sequence.hpp"

#include "utils.h"

#include <string>
#include <iostream>
#include <vector>

enum directive_type{
    DIRECTIVE_TYPE_STRING,
    DIRECTIVE_TYPE_WORD,
    DIRECTIVE_TYPE_ZERO,
    DIRECTIVE_TYPE_ALIGN
};

template <directive_type type>
class Directive :public Sequence{


public:

    Directive(std::string str);
    Directive(uint32_t dat);
    Directive(uint32_t num1,uint32_t num2);
};

template <>
inline Directive<DIRECTIVE_TYPE_STRING>::Directive(std::string str){
    *this << str;
}

template <>
inline Directive<DIRECTIVE_TYPE_WORD>::Directive(uint32_t dat){
    *this << dat;
}

template <>
inline Directive<DIRECTIVE_TYPE_ZERO>::Directive(uint32_t num){
    size_t size = num*sizeof(char);
    std::vector<char> tmp(size,0);
    *this << tmp;
}

//this's not a directive that focus on the memory organization with a partial view
//so a current position from outside should be given
template <>
inline Directive<DIRECTIVE_TYPE_ALIGN>::Directive(uint32_t align,uint32_t pos){
    //check the current pos
    uint32_t nextPos = MOD(pos,align)?(ROUND(pos,align)+(1<<align)):pos;
    size_t size = nextPos - pos;
    std::vector<char> tmp(size,0);
    *this << tmp;
}