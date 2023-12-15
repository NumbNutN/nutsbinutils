#include <elf.h>

#include <unistd.h>

#include <fstream>
#include <vector>
#include <memory>

class shdrtbl{

private:

    struct unit{
        Elf32_Shdr shdr;
        std::unique_ptr<char> dat;
    };

    Elf32_Ehdr& _ehdr;

    //section header table
    std::vector<unit> sectionUnitList;

public:

    shdrtbl(Elf32_Ehdr& ehdr):_ehdr(ehdr){}

    void insert(void* dat,uint32_t addr,uint32_t offset,uint32_t size){
        
        Elf32_Shdr sechdr = {
            .sh_type = SHT_PROGBITS,
            .sh_flags = SHF_WRITE | SHF_ALLOC | SHF_EXECINSTR,
            .sh_addr = addr,
            .sh_offset = offset,
            .sh_size = size
        };

        sectionUnitList.push_back({
            sechdr,
            dat
        });
        _ehdr.e_shnum++;
    }

    //write section header list
    friend std::ostream &operator<<(std::ostream& out,const shdrtbl& stbl);
};

std::ostream &operator<<(std::ostream& out,const shdrtbl& stbl){

    for(auto unit :stbl.sectionUnitList){
        //write section header table
        out.seekp(stbl._ehdr.e_shoff, std::ios::beg);
        out.write(reinterpret_cast<const char*>(&unit.shdr), sizeof(Elf32_Shdr));

        //write each section
        out.seekp(unit.shdr.sh_offset, std::ios::beg);
        out.write((const char*)unit.dat, unit.shdr.sh_size);
    }
    
}