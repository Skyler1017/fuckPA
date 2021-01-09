#include "proc.h"
#include <elf.h>

#ifdef __ISA_AM_NATIVE__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

extern uint8_t ramdisk_start;
extern uint8_t ramdisk_end;
size_t ramdisk_read(void *buf, size_t offset, size_t len);

static uintptr_t loader(PCB *pcb, const char *filename) {
    Elf_Ehdr *ehdr = (Elf_Ehdr *)&ramdisk_start;

    if (!( ehdr->e_ident[EI_MAG0] == ELFMAG0 &&
           ehdr->e_ident[EI_MAG1] == ELFMAG1 &&
           ehdr->e_ident[EI_MAG2] == ELFMAG2 &&
           ehdr->e_ident[EI_MAG3] == ELFMAG3) ) {

        assert(0);
    }

    Elf_Phdr *phdr = (Elf_Phdr *)((char*)ehdr + ehdr->e_phoff);
    int num = ehdr -> e_shnum;
    for(int i = 0; i < num; ++ i ){
        ramdisk_read((void *)phdr->p_vaddr, phdr->p_offset, phdr->p_filesz);
        if(phdr->p_filesz < phdr -> p_memsz){
            memset((void *)phdr->p_vaddr + phdr->p_filesz, 0, phdr->p_memsz - phdr->p_filesz);
        }
        ++ phdr;
    }
    return ehdr->e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
    uintptr_t entry = loader(pcb, filename);
    Log("Jump to entry = %x", entry);
    ((void(*)())entry) ();
}

void context_kload(PCB *pcb, void *entry) {
    _Area stack;
    stack.start = pcb->stack;
    stack.end = stack.start + sizeof(pcb->stack);

    pcb->cp = _kcontext(stack, entry, NULL);
}

void context_uload(PCB *pcb, const char *filename) {
    uintptr_t entry = loader(pcb, filename);

    _Area stack;
    stack.start = pcb->stack;
    stack.end = stack.start + sizeof(pcb->stack);

    pcb->cp = _ucontext(&pcb->as, stack, stack, (void *)entry, NULL);
}