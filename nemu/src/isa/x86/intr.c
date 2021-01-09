#include "rtl/rtl.h"

void raise_intr(uint32_t NO, vaddr_t ret_addr) {
    Assert(NO <= cpu.idtr.limit, "interrupt NO is outof bound.");
    rtl_push(&cpu.eflags.val);
    rtl_push(&cpu.cs);
    rtl_push(&ret_addr);

    cpu.eflags.IF = 0; // disable the interrupt

    uint32_t low = vaddr_read(cpu.idtr.base + 8 * NO, 4) & 0x0000ffff;
    uint32_t high = vaddr_read(cpu.idtr.base + 8 * NO + 4, 4) & 0xffff0000;

    rtl_j(high | low);
}

bool isa_query_intr(void) {
  return false;
}
