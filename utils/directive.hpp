#pragma once

#include "binbuf.hpp"

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
class directive{


private:
    size_t _size;
    binbuf _buf;

public:

    directive(std::string str);
    directive(uint32_t dat);
    directive(uint32_t num1,uint32_t num2);

    const size_t size(){
        return _size;
    }

    template <directive_type type2>
    friend std::ostream& operator<<(std::ostream& out,directive<type2>& directive);
};

template <>
inline directive<STRING>::directive(std::string str){
    std::ostream out(&_buf);
    out << str;
    char tail = '\0';
    _size = str.size() + 1;
    out.write(&tail, 1);
}

template <>
inline directive<WORD>::directive(uint32_t dat){
    std::ostream out(&_buf);
    _size = sizeof(uint32_t);
    out.write((char*)&dat, _size);
}

template <>
inline directive<ZERO>::directive(uint32_t num){
    std::ostream out(&_buf);
    _size = num*sizeof(char);
    char tmp[_size];
    memset(tmp,0,_size);
    out.write(tmp,_size);
}

//this's not a directive that focus on the memory organization with a partial view
//so a current position from outside should be given
template <>
inline directive<ALIGN>::directive(uint32_t align,uint32_t pos){
    //check the current pos
    uint32_t nextPos = MOD(pos,align)?(ROUND(pos,align)+(1<<align)):pos;
    std::ostream out(&_buf);
    _size = nextPos - pos;
    char tmp[_size];
    memset(tmp,0,_size);
    out.write(tmp,_size);
}

template <directive_type type>
inline std::ostream& operator<<(std::ostream& out,directive<type>& directive){
    std::istream in(&directive._buf);
    char tmp[directive.size()];
    in.read(tmp,directive.size());
    out.write(tmp, directive.size());
    return out;
}