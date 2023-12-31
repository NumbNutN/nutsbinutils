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
    char_type* _buf;
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

        //reallocate a larger array
        _size <<= 1;

        if(_size > _maxSize)return traits_type::eof();
        
        setbuf(_buf,_size);
        
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
        setg(_buf, gptr(), end());
        //return the next available character
        return *gptr();
    }

    virtual std::streambuf* setbuf(char* s,std::streamsize n) override{
        if(n < _size)throw std::exception();
        if(n > _maxSize)throw std::exception();

        char_type* lastbuf = s;
        s = (char_type*)realloc(s,n);

        //adjust the put area pointer
        setp(s + (pptr() - lastbuf), s+n);

        //get area pointer also
        setg(s,s + (gptr()-eback()),s + (egptr() - eback()));

        //set the _end mark
        _end = pptr();

        _buf = s;

        _size = n;

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
            ptr = _buf;
            pos = off;
        }
        if(way == std::ios_base::end){
            ptr = end();
            pos = (end() - _buf) + off;
        }
        if((way == std::ios_base::cur) && (mode & std::ios_base::in) == std::ios_base::in){
            ptr = gptr();
            pos = (gptr() - _buf) + off;
        }
        if((way == std::ios_base::cur) && (mode & std::ios_base::out) == std::ios_base::out){
            ptr = pptr();
            pos = (pptr() - _buf) + off;
        }

        if((mode & std::ios_base::in) == std::ios_base::in){
            if(_buf + pos > end())return (pos_type)-1;
            setg(eback(),ptr+off,egptr());
        }
        if((mode & std::ios_base::out) == std::ios_base::out){
            //if the seek position is larger than current epptr()
            //reset the buffer size with the minimum requirement
            if(_buf + pos > epptr()){
                setbuf(_buf,pos);
                seekoff(off,way,mode);
            }
            else setp(ptr+off,epptr());
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
        _buf = (char_type*)malloc(_size);
        //set the get area pointer
        setg(_buf, _buf, _buf);
        //set the put area pointer
        setp(_buf, _buf + _size);
        //set the _end mark
        _end = _buf;
    }

    binbuf(const binbuf& obj):std::streambuf(obj){
        //flush before buffer copy
        std::ostream out((std::streambuf*)&obj);
        out.flush();
        _size = obj._size;
        _buf = (char_type*)malloc(_size);
        //set the _end mark
        _end = obj._end - obj._buf + _buf;
        memcpy(_buf,obj._buf,_size);
        setg(_buf, _buf + (obj.gptr() - obj._buf), _buf + (obj.egptr() - obj._buf));
        setp(_buf + (obj.pptr() - obj._buf),_buf +_size);
    }

    void info(std::ostream& out){
        std::cout << "\033[32mbinary buffer info(in hex)" << std::endl;
        out << std::hex << "buffer base " << (uint64_t)_buf << std::endl;
        out << std::hex << "pbegin " << (uint64_t)pbase() << " pnext " << (uint64_t)pptr() << " pend " << (uint64_t)epptr() << std::endl;
        out << std::hex << "gbegin " << (uint64_t)eback() << " gnext " << (uint64_t)gptr() << " gend " << (uint64_t)egptr() << std::endl;
        std::cout << "\033[0m" << std::endl;
        std::cout << std::endl;
    }

    void content(std::ostream& out){
        char* p;
        const size_t showPerLine = 16;
        out << "\033[34mbinary buffer content(in hex)" << std::endl;
        //write the list
        for(int i=0;i<showPerLine;++i){
            out << std::hex <<std::setfill('0') << std::setw(2) << i;
            if((i + 1) == showPerLine)out << std::endl;
            else out << ' ';
        }

        //start from the gbase
        out << std::hex << std::setfill('0') << std::setw(2) << 0 << ' ';
        for(p = eback();p!= end();++p){
            //write down the hex content
            char dat = *p;
            out << std::hex << std::setfill('0') << std::setw(2)<< (((uint32_t)dat) & 0xFF);
            if((p - eback() + 1)%showPerLine)out << ' ';
            else{
                out << std::endl;
                out << std::hex << std::setfill('0') << std::setw(2) << p + 1 - eback() << ' ';
            }
        }

        if((p - eback())%showPerLine)out << std::endl;
        out << std::endl;

        for(p = eback();p!= end();++p){
            //write down the hex content
            char dat = *p;
            if(dat >= 'A' && dat <= 'z')
                out << std::setfill(' ') << std::setw(2) << dat;
            else 
                out << "..";

            if((p - eback() + 1)%showPerLine)out << ' ';
            else out << std::endl;
        }

        std::cout << "\033[0m" << std::endl;
        if((p - eback())%showPerLine)out << std::endl;
        out << std::endl;
    }

    virtual ~binbuf() {
        free(_buf);
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
