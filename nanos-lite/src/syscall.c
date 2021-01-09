#include "common.h"
#include "syscall.h"
void _yield();
_Context* do_syscall(_Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

  switch (a[0]) {
      case SYS_exit:
          _halt(0);
          return c;
      case SYS_yield :
          _yield();
          return c;
      case SYS_write:{
          int i = 0;
          if(a[1] == 1 || a[1] == 2) {
              char *str = (char *)a[2];
              for(i = 0; i < a[3]; ++ i) _putc(str[i]);
          }
          c->GPRx = i;
          return c;
      }
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
