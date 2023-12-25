#pragma once
#include <stdint.h>

class Sequence{

protected:
    uint32_t _base;
    uint32_t _offset;

public:
    uint32_t pos(){
        return _base + _offset;
    }

    virtual void set_offset(int32_t new_offset){
        _offset = new_offset;
    }

    virtual void set_base(uint32_t new_base){
        _base = new_base;
    }
};