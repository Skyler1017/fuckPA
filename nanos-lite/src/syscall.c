#include "common.h"
#include "syscall.h"
void _yield();
_Context* do_syscall(_Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;

  switch (a[0]) {
      case SYS_exit:
          _halt(0);
          return c;
      case SYS_yield :
          _yield();
          return c;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
