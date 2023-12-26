#pragma once
#include <stdint.h>

#include "sequence.hpp"

#include <vector>

class Container: public Sequence{

private:
    std::vector<Sequence*> _seq_maintain_list;
protected:
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

    //insert a sequence
    void insert(Sequence& obj){
        _seq_maintain_list.push_back(&obj);
    }

    //insert a sequence that really go into effect after a moment
    // void insertLater(Sequence& obj){

    // }

    friend Container& operator<<(Container& seg,Sequence& sec);
};

inline Container& operator<<(Container& seg,Sequence& sec){
    std::ostream out(&seg._buf);
    out << sec;
    return seg;
}