#include <elf.h>

class exculate_file{

private:

    Elf32_Ehdr _ehdr;           /* ELF header */
public:

    exculate_file(Elf32_Ehdr ehdr) :_ehdr(ehdr){}



};