#pragma once
#include <stdint.h>

#include <vector>

#include "sequence.hpp"

#include "utils.h"

template <uint32_t align>
class Container: public Sequence{

private:
    std::vector<Sequence*> _seq_maintain_list;

    uint32_t alloc_offset(uint32_t size){

        //first check if _off is align
        _poff = MOD(_poff,align)?(ROUND(_poff,align)+(1<<align)):_poff;

        //count a new size
        _size = _poff + size;
        //align the request size
        uint32_t tmp = _poff;
        uint32_t sz = MOD(size,align)?(ROUND(size,align)+(1<<align)):size;
        _poff += sz;

        return tmp;
    }

protected:
    uint32_t _poff = 0;

    //set the offset of self
    //reset the offset
    virtual void set_base(uint32_t new_base){

        Sequence::set_base(new_base);
        for(Sequence* seq:_seq_maintain_list){
            seq->set_base(pos());
        }
    }

    virtual void set_offset(int32_t new_offset){
        Sequence::set_offset(new_offset);
        for(Sequence* seq:_seq_maintain_list){
            seq->set_base(pos());
        }        
    }

    uint32_t get_cur_offset(){
        return _poff;
    }
public:
    void refresh(Sequence& seq){
        std::ostream out(&buffer());
        out.seekp(seq.get_offset());
        out << seq;
    }

    void refreshAll(){
        for(Sequence* pseq:_seq_maintain_list){
            refresh(*pseq);
        }
    }

    template <uint32_t align2>
    friend Container<align2>& operator<<(Container<align2>& seg,Sequence& sec);
};

template <uint32_t align>
inline Container<align>& operator<<(Container<align>& ctn,Sequence& seq){
    ctn._seq_maintain_list.push_back(&seq);
    uint32_t off = ctn.alloc_offset(seq.size());
    seq.set_offset(off);
    seq.set_base(ctn.pos());

    //refresh the current sequence before insert
    // ctn.refresh(seq);
    std::ostream out(&ctn._buf);
    out.seekp(off);
    out << seq;

    return ctn;
}