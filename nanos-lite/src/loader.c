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

extern uint8_t ramdisk_start;
extern uint8_t ramdisk_end;
size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);
static uintptr_t loader(PCB *pcb, const char *filename) {
    int fd = fs_open(filename, 0, 0);
    Elf_Ehdr ehdr;
    fs_read(fd, (void *)&ehdr, sizeof(Elf_Ehdr));

    // 检查文件格式
    if (ehdr.e_ident[EI_MAG0] != ELFMAG0 ||
        ehdr.e_ident[EI_MAG1] != ELFMAG1 ||
        ehdr.e_ident[EI_MAG2] != ELFMAG2 ||
        ehdr.e_ident[EI_MAG3] != ELFMAG3)
    {
        Log("invalid file formate");
        assert(0);
    }

    // Elf_Phdr段信息描述符
    Elf_Phdr phdr;
    // 段数量
    int num = ehdr.e_shnum;
    for (int i = 0; i < num; ++i)
    {
        // 从ramdisk中`offset`偏移处的`len`字节读入到`buf`中
        // size_t ramdisk_read(void *buf, size_t offset, size_t len);
        fs_lseek(fd, ehdr.e_phoff + i * ehdr.e_phentsize, SEEK_SET);
        fs_read(fd, &phdr, sizeof(Elf_Phdr));
        if (phdr.p_type != PT_LOAD)
            continue;
        fs_lseek(fd, phdr.p_offset, SEEK_SET);
        fs_read(fd, (void *)phdr.p_vaddr, phdr.p_filesz);
        // 内存位置置wei0
        memset((void *)phdr.p_vaddr + phdr.p_filesz, 0, phdr.p_memsz - phdr.p_filesz);
    }

    fs_close(fd);
    // 虚拟地址
    return ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
    printf("load %s\n",filename);
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