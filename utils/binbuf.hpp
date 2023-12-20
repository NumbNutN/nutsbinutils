#pragma once

#include <streambuf>

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

    //should be override by derived class
    //set the pointers refered the relative position and return a absolute position
    virtual pos_type
      seekoff(off_type off, std::ios_base::seekdir way,
	      std::ios_base::openmode mode/*__mode*/ = std::ios_base::in | std::ios_base::out) override{
        
        //the base address
        char_type* ptr;
        //the absolute position
        pos_type pos;

        if(way == std::ios_base::beg){
            ptr = buf;
            pos = off;
        }
        if(way == std::ios_base::end){
            ptr = pptr();
            pos = (pptr() - buf) + off;
        }
        if((way == std::ios_base::cur) && (mode & std::ios_base::in) == std::ios_base::in){
            ptr = gptr();
            pos = (gptr() - buf) + off;
        }
        if((way == std::ios_base::cur) && (mode & std::ios_base::out) == std::ios_base::out){
            ptr = pptr();
            pos = (pptr() - buf) + off;
        }

        if(buf + pos >= pptr())return (pos_type)-1;
        if((mode & std::ios_base::in) == std::ios_base::in){
            setg(eback(),ptr+off,egptr());
        }
        if((mode & std::ios_base::out) == std::ios_base::out){
            setp(ptr+off,epptr());
        }
        return pos;
    }

    //should be override by derived class
    //set the pointers refered the absolute position
    virtual pos_type
      seekpos(pos_type off, std::ios_base::openmode mode/*__mode*/ = std::ios_base::in | std::ios_base::out) override{
        return seekoff(off, std::ios_base::beg,mode);
    }

public:
    binbuf() {
        //allocate the base buffer
        buf = (char_type*)malloc(_size);
        setbuf(buf,_size);
    }

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
