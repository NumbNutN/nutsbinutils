#pragma once

#include <streambuf>
#include <iostream>
#include <iomanip>

#include <memory.h>
#include <stdlib.h>

// a binary buffer maintain a sequential memory
// _maxsize: the max length the buffer can extend
// _size: current buffer length, equal to epptr()
// pptr(): the current put area pointer, but sometimes doesn't mark the end of buffer, if so, compare it with _end and return a higher value
// _end: the farthest position that put area pointer reached, it update when overflow or seekoff event occus
// gptr(): a EOF will be return if gptr() farther than end()
// WARN: a seekp that with a address higher than _end is not allowed, and will cause buffer state abnormal
class binbuf : public std::streambuf{

private:
    size_t _size = 2;
    size_t _maxSize = 4096;
    char_type* buf;
    size_t underflowCnt = 0;
    size_t overflowCnt = 0;
    char_type* _end;

    //the maxinum of _end and pptr is always the EOF
    char_type* end(){
        return _end > pptr()? _end:pptr();
    }

protected:
    virtual int_type overflow(int_type c) override{
        //std::cout << "overflow count " << ++overflowCnt << std::endl;
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

        //remember to give _end a new value when the array change its base
        //abviously, when overflow happen, pptr is always the farthest position ever before
        _end = pptr();

        //just return something else
        return c;
    }

    virtual int_type underflow() override {

        //std::cout << "underflow count " << ++underflowCnt << std::endl;

        //the end() marks the EOF
        if(gptr() == end())return traits_type::eof();
        setg(buf, gptr(), end());
        //return the next available character
        return *gptr();
    }

    virtual std::streambuf* setbuf(char* s,std::streamsize n) override{
        //set the get area pointer
        setg(buf, buf, buf);
        //set the put area pointer
        setp(buf, buf + n);
        //set the _end mark
        _end = buf;

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

        //refresh the _end mark
        _end = end();

        if(way == std::ios_base::beg){
            ptr = buf;
            pos = off;
        }
        if(way == std::ios_base::end){
            ptr = end();
            pos = (end() - buf) + off;
        }
        if((way == std::ios_base::cur) && (mode & std::ios_base::in) == std::ios_base::in){
            ptr = gptr();
            pos = (gptr() - buf) + off;
        }
        if((way == std::ios_base::cur) && (mode & std::ios_base::out) == std::ios_base::out){
            ptr = pptr();
            pos = (pptr() - buf) + off;
        }

        if((mode & std::ios_base::in) == std::ios_base::in){
            if(buf + pos > end())return (pos_type)-1;
            setg(eback(),ptr+off,egptr());
        }
        if((mode & std::ios_base::out) == std::ios_base::out){
            if(buf + pos > epptr())return (pos_type)-1;
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

    binbuf(const binbuf& obj):std::streambuf(obj){
        //flush before buffer copy
        std::ostream out((std::streambuf*)&obj);
        out.flush();
        _size = obj._size;
        buf = (char_type*)malloc(_size);
        //set the _end mark
        _end = obj._end - obj.buf + buf;
        memcpy(buf,obj.buf,_size);
        setg(buf, buf + (obj.gptr() - obj.buf), buf + (obj.egptr() - obj.buf));
        setp(buf + (obj.pptr() - obj.buf),buf +_size);
    }

    void info(std::ostream& out){
        std::cout << "\033[32mbinary buffer info(in hex)" << std::endl;
        out << std::hex << "buffer base " << (uint64_t)buf << std::endl;
        out << std::hex << "pbegin " << (uint64_t)pbase() << " pnext " << (uint64_t)pptr() << " pend " << (uint64_t)epptr() << std::endl;
        out << std::hex << "gbegin " << (uint64_t)eback() << " gnext " << (uint64_t)gptr() << " gend " << (uint64_t)egptr() << std::endl;
        std::cout << std::endl;
    }

    void content(std::ostream& out){
        char* p;
        const size_t showPerLine = 16;
        out << "\033[32mbinary buffer content(in hex)" << std::endl;
        //write the list
        for(int i=0;i<showPerLine;++i){
            out << std::hex <<std::setfill('0') << std::setw(2) << i;
            if((i + 1) == showPerLine)out << std::endl;
            else out << ' ';
        }

        //start from the gbase
        for(p = eback();p!= end();++p){
            char dat = *p;
            out << std::hex << std::setfill('0') << std::setw(2)<< (((uint32_t)dat) & 0xFF);
            if((p - eback() + 1)%showPerLine)out << ' ';
            else out << std::endl;
        }
        if((p - eback())%showPerLine)out << std::endl;
        out << std::endl;
    }

    virtual ~binbuf() {
        free(buf);
    }

    friend std::ostream& operator<<(std::ostream& out,binbuf& buf);
};

/* 
 * take a view of binary buffer
 * only used for debug 
*/
inline std::ostream& operator<<(std::ostream& out,binbuf& buf){
    buf.info(out);
    buf.content(out);
    return out;
}
