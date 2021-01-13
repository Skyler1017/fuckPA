#include "proc.h"
#include "fs.h"
#include <elf.h>


#ifdef __ISA_AM_NATIVE__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

typedef uint32_t PTE;
typedef uint32_t PDE;

#define PDX(va)          (((uint32_t)(va) >> PDXSHFT) & 0x3ff)
#define PTX(va)          (((uint32_t)(va) >> PTXSHFT) & 0x3ff)
#define OFF(va)          ((uint32_t)(va) & 0xfff)
#define ROUNDUP(a, sz)   ((((uintptr_t)a)+(sz)-1) & ~((sz)-1))
#define ROUNDDOWN(a, sz) ((((uintptr_t)a)) & ~((sz)-1))
#define PTE_ADDR(pte)    ((uint32_t)(pte) & ~0xfff)
#define PGSIZE 4096
#define PTE_P          0x001   // Present
#define PGSHFT         12      // log2(PGSIZE)
#define PTXSHFT        12      // Offset of PTX in a linear address
#define PDXSHFT        22      // Offset of PDX in a linear address

#define MIN(a, b) (a < b ? a : b)

extern uint8_t ramdisk_start;
extern uint8_t ramdisk_end;

size_t ramdisk_read(void *buf, size_t offset, size_t len);

void context_uload(PCB *pcb, const char *filename);

PCB *pcb_load;

void load_single(int fd, PCB *pcb, uint32_t va, uint32_t bin_size, uint32_t sgsize) {
    //Log("va: 0x%08x, bin_size: %d, sgsize: %d", va, bin_size, sgsize);

    uint32_t offset = va - ROUNDDOWN(va, PGSIZE);
    uint32_t size = 0;
    // 不对齐要单独处理
    if (offset > 0) {
        size = PGSIZE - offset;
        void *pa = new_page(1);
        // va和va - offset 都行，用不到低12位
        _map(&(pcb->as), (void *) (uintptr_t) va, pa, PTE_P);
        fs_read(fd, pa, MIN(size, bin_size));
    }
    uint32_t i;
    for (i = size; i < bin_size; i += PGSIZE) {
        void *pa = new_page(1);
        _map(&(pcb->as), (void *) (uintptr_t)(va + i), pa, PTE_P);
        fs_read(fd, pa, MIN(PGSIZE, bin_size - i));
    }
    // 物理页填充0
    while (i < sgsize) {
        void *pa = new_page(1);
        _map(&(pcb->as), (void *) (uintptr_t)(va + i), pa, PTE_P);
        memset((void *) (uintptr_t) pa, 0, PGSIZE);
        i += PGSIZE;
    }

}

static uintptr_t loader(PCB *pcb, const char *filename) {
    _protect(&(pcb->as));
    int fd = fs_open(filename, 0, 0);
    Elf_Ehdr ehdr;
    fs_lseek(fd, 0, SEEK_SET);
    fs_read(fd, (void *) &ehdr, sizeof(Elf_Ehdr));

    // check whether the file format is ELF
    if (!(ehdr.e_ident[EI_MAG0] == ELFMAG0 &&
          ehdr.e_ident[EI_MAG1] == ELFMAG1 &&
          ehdr.e_ident[EI_MAG2] == ELFMAG2 &&
          ehdr.e_ident[EI_MAG3] == ELFMAG3)) {

        assert(0);
    }


    Elf_Phdr phdr;
    int num = ehdr.e_shnum;
    for (int i = 0; i < num; ++i) {
        //Log("start read Phdr");
        fs_lseek(fd, ehdr.e_phoff + i * ehdr.e_phentsize, SEEK_SET);
        fs_read(fd, &phdr, sizeof(Elf_Phdr));
        //Log("read Phdr success");
        if (phdr.p_type != PT_LOAD) continue;

        fs_lseek(fd, phdr.p_offset, SEEK_SET);
        load_single(fd, pcb, phdr.p_vaddr, phdr.p_filesz, phdr.p_memsz);
        Log("load success. va: [0x%08x, 0x%08x)", phdr.p_vaddr, phdr.p_vaddr + phdr.p_memsz);
        //memset((void *)phdr.p_vaddr + phdr.p_filesz, 0, phdr.p_memsz - phdr.p_filesz);
    }

    fs_close(fd);
    Log("load %s success. PCB: 0x%08x", filename, pcb);
    return ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
    printf("load %s\n", filename);
    uintptr_t entry = loader(pcb, filename);
    Log("Jump to entry = %x", entry);
    ((void (*)()) entry)();
}

void context_kload(PCB *pcb, void *entry) {
    _Area stack;
    stack.start = pcb->stack;
    stack.end = stack.start + sizeof(pcb->stack);

    pcb->cp = _kcontext(stack, entry, NULL);
    _protect(&(pcb->as));
    pcb->cp->as = &(pcb->as);
}

void context_uload(PCB *pcb, const char *filename) {
    uintptr_t entry = loader(pcb, filename);
    _Area stack;
    stack.start = pcb->stack;
    stack.end = stack.start + sizeof(pcb->stack);

    pcb->cp = _ucontext(&pcb->as, stack, stack, (void *) entry, NULL);
}