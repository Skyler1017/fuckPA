#include "cpu/exec.h"

make_EHelper(mov);

make_EHelper(operand_size);

make_EHelper(inv);
make_EHelper(nemu_trap);
// system.c
make_EHelper(out);
make_EHelper(in);
make_EHelper(lidt);
make_EHelper(mov_r2cr);
make_EHelper(mov_cr2r);

make_EHelper(int);

make_EHelper(call);            // control.c
make_EHelper(call_rm);
make_EHelper(jmp_rm);
make_EHelper(jmp);
make_EHelper(ret);
make_EHelper(iret);
make_EHelper(jcc);


make_EHelper(push);        //data-mov.c
make_EHelper(pusha);
make_EHelper(pop);
make_EHelper(popa);
make_EHelper(lea);
make_EHelper(leave);
make_EHelper(movsb);
make_EHelper(movsl);
make_EHelper(movzx);
make_EHelper(movsx);
make_EHelper(cltd); // CWD (opcode 99)
make_EHelper(cwtl); // CBW (opcode 98)


make_EHelper(sub);            //arith.c
make_EHelper(add);
make_EHelper(adc);
make_EHelper(sbb);
make_EHelper(cmp);
make_EHelper(imul3);
make_EHelper(imul2);
make_EHelper(imul1);
make_EHelper(div);
make_EHelper(idiv);
make_EHelper(mul);
make_EHelper(neg);
make_EHelper(inc);
make_EHelper(dec);

make_EHelper(xor);            //logic.c
make_EHelper(or);
make_EHelper(and);
make_EHelper(rol);
make_EHelper(shl);
make_EHelper(shr);
make_EHelper(sar);
make_EHelper(test);
make_EHelper(not);
make_EHelper(setcc);


//speical.c
make_EHelper(nop);  
