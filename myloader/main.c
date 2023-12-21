#include <unistd.h>
//open
#include <fcntl.h>
//assert
#include <assert.h>
//elf
#include <elf.h>
//mmap
#include <sys/mman.h>
//perror
#include <stdio.h>

#include "utils.h"

void my_execve(const char* file,char* argv[],char *envp[]){
   
    //read ELF format file
    int fd = open(file,O_RDONLY);
    assert(fd >= 0 && "file is not existed");

    //print some architecture info
    int res;
    res = sysconf(_SC_PAGE_SIZE);
    printf("align size require:%d\n",res);

    Elf32_Ehdr* h = (Elf32_Ehdr*)mmap(NULL,4096,PROT_READ,MAP_PRIVATE,fd,0);
    assert(h != MAP_FAILED);
    assert((h->e_type == ET_EXEC || h->e_type == ET_REL) && h->e_machine == EM_ARM);

    //program segment header table
    Elf32_Phdr* pht = (Elf32_Phdr*)((char*)h + h->e_phoff);
    //load all the LOAD segments
    for(int i=0;i<h->e_phnum;++i){
        Elf32_Phdr* p = &pht[i];
        if(p->p_type == PT_LOAD){
            //memory map region consider alignment
            uintptr_t map_beg = ROUND(p->p_vaddr,p->p_align);
            uintptr_t map_end = map_beg + p->p_memsz;
            map_end = MOD(map_end,p->p_align)? (ROUND(map_end,p->p_align) + (1<<p->p_align)):(map_end);

            //memory map flags
            int prot = 0;
            if(p->p_flags & PF_R)prot |= PROT_READ;
            if(p->p_flags & PF_W)prot |= PROT_WRITE;
            if(p->p_flags & PF_X)prot |= PROT_EXEC;

            //memory map size
            int map_sz = p->p_filesz + MOD(p->p_vaddr,p->p_align);
            map_sz = MOD(map_sz,p->p_align)?(ROUND(map_sz,p->p_align)+(1<<p->p_align)):map_sz;
	    
            int offset = ROUND(p->p_offset,p->p_align);
            //map file content to memory
            void* ret = mmap(
                (void*)map_beg,
                map_sz,
                prot,
                MAP_PRIVATE|MAP_FIXED,
                fd,
                offset
            );
            assert(ret != MAP_FAILED);

            //map extra anonymous memory
            intptr_t extra_sz = p->p_memsz - p->p_filesz;
            if(extra_sz){
                uintptr_t extra_beg = map_beg + map_sz;
                ret = mmap(
                    (void*)extra_beg,extra_sz,prot,
                    MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED,
                    -1,
                    0
                );
                assert(ret != MAP_FAILED);
            }
        }
    }
    close(fd);

    //now just cast the entry point address to a function pointer and call it
    void (*entry_)() = (void(*)())h->e_entry;
    entry_();
}

int main(int argc,char* argv[],char* envp[]){
    
    my_execve(argv[1], argv + 1, envp);
}
