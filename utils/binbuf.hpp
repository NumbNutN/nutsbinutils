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
    size_t underflowCnt = 0;
    size_t overflowCnt = 0;

protected:
    virtual int_type overflow(int_type c) override{
        std::cout << "overflow count " << ++overflowCnt << std::endl;
        if(_size >= _maxSize)return traits_type::eof();

        //reallocate a larger array
        _size <<= 1;
        char_type* lastbuf = buf;
        buf = (char_type*)realloc(buf,_size);

        //adjust the put area pointer
        setp(buf + (pptr() - lastbuf), buf+_size);

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

        std::cout << "underflow count " << ++underflowCnt << std::endl;
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

    virtual pos_type
      seekoff(off_type off, std::ios::seekdir way,
	      std::ios::openmode mode/*__mode*/ = std::ios::in | std::ios::out) override{
        
        //the base address
        char_type* ptr;
        //the absolute position
        pos_type pos;

        if(way == std::ios::beg){
            ptr = buf;
            pos = off;
        }
        if(way == std::ios::end){
            ptr = pptr();
            pos = (pptr() - buf) + off;
        }
        if((way == std::ios::cur) && (mode & std::ios::in) == std::ios::in){
            ptr = gptr();
            pos = (gptr() - buf) + off;
        }
        if((way == std::ios::cur) && (mode & std::ios::out) == std::ios::out){
            ptr = pptr();
            pos = (pptr() - buf) + off;
        }

        if(buf + pos >= pptr())return (pos_type)-1;
        if((mode & std::ios::in) == std::ios::in){
            setg(eback(),ptr+off,egptr());
        }
        if((mode & std::ios::out) == std::ios::out){
            setp(ptr+off,epptr());
        }
        return pos;
    }

    virtual pos_type
      seekpos(pos_type off, std::ios::openmode mode/*__mode*/ = std::ios::in | std::ios::out) override{
        return seekoff(off, std::ios::beg,mode);
    }

public:
    binbuf() {
        //allocate the base buffer
        buf = (char_type*)malloc(_size);
        setbuf(buf,_size);
    }

    // template <std::streambuf& streambuf>
    // concept flushed_streambuf = streambuf.pbase() == streambuf.pptr();

    binbuf(binbuf& obj):std::streambuf(obj){
        //binbuf should be flush before copy constructor
        std::ostream out((std::streambuf*)&obj);
        out.flush();
        _size = obj._size;
        buf = (char_type*)malloc(_size);
        memcpy(buf,obj.buf,_size);
        setg(buf, buf + (obj.gptr() - obj.buf), buf + (obj.egptr() - obj.buf));
        setp(buf + (obj.pptr() - obj.buf),buf +_size);
    }

    size_t length() const{
        size_t pa_size = pptr() - buf;
        size_t ga_size = gptr() - eback();
        return pa_size - ga_size;
    }

    void info(std::ostream& out){
        out << std::hex << "buffer base " << (uint64_t)buf << std::endl;
        out << std::hex << "pbegin " << (uint64_t)pbase() << " pnext " << (uint64_t)pptr() << " pend " << (uint64_t)epptr() << std::endl;
        out << std::hex << "gbegin " << (uint64_t)eback() << " gnext " << (uint64_t)gptr() << " gend " << (uint64_t)egptr() << std::endl;
    }

    void* getBase(){
        return buf;
    }

    virtual ~binbuf() {
        free(buf);
    }
};
