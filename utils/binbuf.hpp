#pragma once

#include <streambuf>
#include <iostream>

class binbuf : public std::streambuf{

private:
    size_t _size = 2;
    size_t _maxSize = 4096;
    char* buf;

protected:
    virtual int_type overflow(int_type c) override{
        if(_size >= _maxSize)return traits_type::eof();

        //reallocate a larger array
        delete [] buf;
        _size <<= 1;
        buf = new char(_size);

        //adjust the end pointer
        setp(buf, buf+_size);
        //insert the new character
        *pptr() = c;
        pbump(1);

        //just return something else
        return c;
    }

    //implemention do not have definition lead to link error

    //use default underflow behaviour
    //virtual int_type underflow();

    virtual std::streambuf* setbuf(char* s,std::streamsize n) override{
        //set the get area pointer
        setg(buf, buf, buf);
        //set the put area pointer
        setp(buf, buf + n);
        return this;
    }
public:
    binbuf() {
        //allocate the base buffer
        buf = new char(_size);
        setbuf(buf,_size);
    }

    binbuf(const binbuf& obj){
        _size = obj._size;
        buf = new char(obj._size);
        setbuf(buf,_size);
    }

    size_t length() const{
        return (size_t)(pptr() - gptr());
    }

    virtual ~binbuf() {
        delete[] buf;
    }
};
