#pragma once
#include <stdint.h>

#include "binbuf.hpp"

class Sequence{

protected:

    uint32_t _base = 0;
    int32_t _offset = 0;
    uint32_t _size = 0;
    binbuf _buf;

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

    virtual uint32_t size(){
        return _size;
    }

    binbuf& buffer(){
        //buffer must be flush before return
        //else it will get problem when copy
        std::ostream out(&_buf);
        out.flush();
        return _buf;
    }

    template <typename T>
    friend Sequence& operator<<(Sequence&,const T);

    friend Sequence& operator<<(Sequence&,const std::string&);

    friend std::ostream& operator<<(std::ostream&,Sequence&);
};


template <typename T>
inline Sequence& operator<<(Sequence& seq,const T dat){
    std::ostream out(&seq._buf);
    out.write((const char*)&dat,sizeof(T));
    seq._size += sizeof(T);
    return seq;
}


inline Sequence& operator<<(Sequence& seq,const std::string& dat){
    std::ostream out(&seq._buf);
    out << dat;
    char c = '\0';
    out.write(&c,sizeof(char));
    seq._size += dat.size() + 1;
    return seq;
}

inline std::ostream& operator<<(std::ostream& out,Sequence& seq)
{
    std::istream in(&seq._buf);
    char tmp[seq.size()];
    in.read(tmp,seq.size());
    out.write(tmp, seq.size());
    out.flush();
    return out;
}