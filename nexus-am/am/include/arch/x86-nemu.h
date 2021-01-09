#ifndef __ARCH_H__
#define __ARCH_H__

struct _Context {
    struct _AddressSpace *as;
    // PUSHA指令压入32位寄存器
    // 入栈顺序:EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI
    // 查看汇编指令可以得出寄存器布局
    uintptr_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    int irq;
    uintptr_t eip, cs, eflags;
};

#define GPR1 eax
#define GPR2 ebx
#define GPR3 ecx
#define GPR4 edx
#define GPRx eax

#endif