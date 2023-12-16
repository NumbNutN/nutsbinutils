#pragma once

#include <streambuf>
#include <iostream>

#include <memory.h>
#include <stdlib.h>

class binbuf : public std::streambuf{

private:
    size_t _size = 2;
    size_t _maxSize = 4096;
    char_type* buf;

protected:
    virtual int_type overflow(int_type c) override{
        if(_size >= _maxSize)return traits_type::eof();

        //reallocate a larger array
        _size <<= 1;
        char_type* lastbuf = buf;
        buf = (char_type*)realloc(buf,_size);

        std::cout << std::hex << "buf " << (uint64_t)lastbuf << " pbegin " << (uint64_t)pbase() << " pnext " << (uint64_t)pptr() << " pend " << (uint64_t)epptr() << std::endl;

        //adjust the put area pointer
        setp(buf + (pptr() - lastbuf), buf+_size);

        std::cout << std::hex << "buf " << (uint64_t)buf << " pbegin " << (uint64_t)pbase() << " pnext " << (uint64_t)pptr() << " pend " << (uint64_t)epptr() << std::endl;

        //get area pointer also
        setg(buf,buf + (gptr()-eback()),buf + (egptr() - eback()));
        
        //insert the new character
        *pptr() = c;
        pbump(1);

        //just return something else
        return c;
    }

    //implemention do not have definition lead to link error

    virtual int_type underflow() override {
        if(gptr() == pptr())return traits_type::eof();
        setg(buf, gptr(), pptr());

        //return the next available character
        return *gptr();
    }

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
        buf = (char_type*)malloc(_size);
        setbuf(buf,_size);
    }

    // template <std::streambuf& streambuf>
    // concept flushed_streambuf = streambuf.pbase() == streambuf.pptr();

    binbuf(const binbuf& obj):std::streambuf((std::streambuf&)obj){
        _size = obj._size;
        buf = (char_type*)malloc(_size);
        memcpy(buf,obj.buf,_size);
        setg(buf, buf + (obj.gptr() - obj.buf), buf + (obj.egptr() - obj.buf));
        setp(buf + (obj.pptr() - obj.buf),buf +_size);

    }

    size_t length() const{
        char_type* pnext = pptr();
        size_t pa_size = pptr() - buf;
        size_t ga_size = gptr() - eback();
        return pa_size - ga_size;
    }

    virtual ~binbuf() {
        free(buf);
    }
};
