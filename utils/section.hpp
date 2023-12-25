#pragma once

#include "container.hpp"
#include "binbuf.hpp"
#include "directive.hpp"
#include "elf.hpp"

#include <elf.h>
#include <iostream>
#include <streambuf>
#include <fstream>


class Section : public Container{

private:
    std::string _name;
    Elf32_Shdr _sechdr;  /* section header */
    binbuf _buf;

    uint32_t sec_ndx;
    bool _isbinding = false;

public:
// container interface
    void set_base(uint32_t new_base){
        
        Container::set_base(new_base);
        //the absolute address of section
        _sechdr.sh_offset = pos();
    }

    void set_offset(int32_t new_offset){

        Container::set_offset(new_offset);
        //the absolute address of section
        _sechdr.sh_offset = pos();
    }


protected:
    const elf& _elfbase;
    Section(const elf& elfbase,const std::string& name,Elf32_Word type,Elf32_Word addr,Elf32_Word flags = 0) :
            _name(name),
            _sechdr({
                .sh_type = type,
                .sh_flags = flags,
                .sh_addr = addr,
                .sh_size = 0
                }),
            _elfbase(elfbase){}

    /*
     * create a section object using section header,basically use when reading from file
     * name is required when construct from a relocatable file
    */
    Section(const elf& elfbase,const std::string& name,const Elf32_Shdr& sechdr): _name(name),_sechdr(sechdr),_elfbase(elfbase){}

public:

    const Elf32_Shdr& getHeader() const{
        return _sechdr;
    }

    void setHeader(const Elf32_Shdr& shdr){
        _sechdr = shdr;
    }

    const elf& getElfbase() const{
        return _elfbase;
    }

    //set ndx
    void setNdx(uint32_t ndx){
        sec_ndx = ndx;
        _isbinding = true;
    }

    const std::string& getName() const{
        return _name;
    }

    uint32_t size() const{
        return _sechdr.sh_size;
    }

    /*
     * the derived have the voluntary to refresh size
    */
    uint32_t& size(){
        return _sechdr.sh_size;
    }

    binbuf& buffer(){
    
        std::ostream out(&_buf);
        //buffer must be flush before return
        //else it will get problem when copy
        out.flush();
        return _buf;
    }

    void setNameIdx(uint32_t idx){
        _sechdr.sh_name = idx;
    }

    uint32_t getNameIdx(){
        return _sechdr.sh_name;
    }

    uint32_t getOffset()const{
        return _sechdr.sh_offset;
    }

    void setAddr(uint32_t addr){
        _sechdr.sh_addr = addr;
    }

    uint32_t flags() const{
        return _sechdr.sh_flags;
    }

    friend std::ostream& operator<<(std::ostream& out,Section& sec);
    friend std::ofstream& operator<<(std::ofstream& out,Section& sec);
    friend std::istream& operator>>(std::istream& in,Section& sec);
    friend std::ifstream& operator>>(std::ifstream& in,Section& sec);

    template <typename T>
    friend Section& operator<<(Section& sec,const T dat);

    friend Section& operator<<(Section& sec,const std::string& dat);

    template<directive_type type>
    friend Section& operator<<(Section& sec,Directive<type>& directive);

};

// now section operator<< has no right to set put area offset
inline std::ostream& operator<<(std::ostream& out,Section& sec)
{
    std::istream in(&sec._buf);
    std::cout << sec._buf;
    char tmp[sec.size()];
    in.read(tmp,sec.size());
    out.write(tmp, sec.size());
    out.flush();
    return out;
}

/**
 * the input stream is usually a binary stream from file
*/
inline std::istream& operator>>(std::istream& in,Section& sec){

    std::ostream out(&sec._buf);
    char tmp[sec._sechdr.sh_size];
    in.read(tmp, sec._sechdr.sh_size);
    out.write(tmp, sec._sechdr.sh_size);

    return in;
}

/**
 * when a input is from a ELF, use this
*/
inline std::ifstream& operator>>(std::ifstream& in,Section& sec){
    if(!sec._isbinding)throw std::exception();
    //seek the section header
    uint32_t pos = sec._elfbase.getSecHdrBase() + sec.sec_ndx*sizeof(Elf32_Shdr);
    in.seekg(pos);
    in.read((char*)&sec._sechdr,sizeof(Elf32_Shdr));
    //seek the section offset
    in.seekg(sec._sechdr.sh_offset);
    (std::istream&)in >> sec;
    return in;
}

/**
 * when section to a ELF, use this
*/
inline std::ofstream& operator<<(std::ofstream& out,Section& sec){
    if(!sec._isbinding)throw std::exception();
    //seek the section header
    uint32_t pos = sec._elfbase.getSecHdrBase() + sec.sec_ndx*sizeof(Elf32_Shdr);
    out.seekp(pos);
    out.write((char*)&sec._sechdr,sizeof(Elf32_Shdr));
    //seek the section offset
    out.seekp(sec._sechdr.sh_offset);
    (std::ostream&)out << sec;
    return out;
}

template <typename T>
inline Section& operator<<(Section& sec,const T dat){
    std::ostream out(&sec._buf);
    out.write((const char*)&dat,sizeof(T));
    return sec;
}


inline Section& operator<<(Section& sec,const std::string& dat){
    std::ostream out(&sec._buf);
    out << dat;
    char c = '\0';
    out.write(&c,sizeof(char));
    return sec;
}

template<directive_type type>
inline Section& operator<<(Section& sec,Directive<type>& directive){
    std::ostream out(&sec._buf);
    out << directive;
    return sec;
}