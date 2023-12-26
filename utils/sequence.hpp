#pragma once
#include <stdint.h>

class Sequence{

protected:
    uint32_t _base = 0;
    int32_t _offset = 0;

public:
    uint32_t pos() const{
        return _base + _offset;
    }

    virtual void set_offset(int32_t new_offset){
        _offset = new_offset;
    }

    virtual void set_base(uint32_t new_base){
        _base = new_base;
    }
};