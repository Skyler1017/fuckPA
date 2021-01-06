#include "cpu/exec.h"

make_EHelper(mov)
        {
                operand_write(id_dest, &id_src->val);
        print_asm_template2(mov);
        }

make_EHelper(push)
        {
                rtl_push(&id_dest->val);
        print_asm_template1(push);
        }

make_EHelper(pop)
        {
                rtl_pop(&id_dest->val);
        operand_write(id_dest, &id_dest->val);
        print_asm_template1(pop);
        }

make_EHelper(pusha)
        {

                s0 = reg_l(R_ESP);
        rtl_push(&reg_l(R_EAX));
        rtl_push(&reg_l(R_ECX));
        rtl_push(&reg_l(R_EDX));
        rtl_push(&reg_l(R_EBX));
        rtl_push(&s0);
        rtl_push(&reg_l(R_EBP));
        rtl_push(&reg_l(R_ESI));
        rtl_push(&reg_l(R_EDI));

        print_asm("pusha");
        }

make_EHelper(popa)
        {
                rtl_pop(&reg_l(R_EDI));
        rtl_pop(&reg_l(R_ESI));
        rtl_pop(&reg_l(R_EBP));
        reg_l(R_ESP) += 4; //  Skip next 4 bytes of stack
        rtl_pop(&reg_l(R_EBX));
        rtl_pop(&reg_l(R_EDX));
        rtl_pop(&reg_l(R_ECX));
        rtl_pop(&reg_l(R_EAX));

        print_asm("popa");
        }

make_EHelper(leave)
        {
                // check oprand_size ?
                rtl_mv(&reg_l(R_ESP), &reg_l(R_EBP));
        rtl_pop(&reg_l(R_EBP));
        print_asm("leave");
        }
/* 
 将%eax寄存器的值符号扩展32位到%edx寄存器，
 如果%eax寄存器的二进制序列的最高位为0，
 则cltd指令将把%edx置为32个0，相反，如果%eax寄存器的二进制序列最高位为1，
 则cltd指令将会自从填充%edx寄存器为32个1
*/
make_EHelper(cltd)
        {
                if (decinfo.isa.is_operand_size_16)
                {
                    //符号拓展
                    rtl_sext(&s0, &reg_l(R_EAX), 2);
                    rtl_sari(&s0, &s0, 16);
                    rtl_sr(R_DX, &s0, 2);
                }
                else
                {
                    rtl_sari(&reg_l(R_EDX), &reg_l(R_EAX), 31);
                }

                print_asm(decinfo.isa.is_operand_size_16 ? "cwtl" : "cltd");
        }
// 将Ax中的有符号数经32位扩展到Eax中，即用AX的符号位填充EAX的高16位。
// 若AX中最高位为0，则将EAX高16位全清0若AX中最高位为1，则将EAX高16位全置1
make_EHelper(cwtl)
        {
                if (decinfo.isa.is_operand_size_16)
                {
                    rtl_lr(&s0, R_AX, 1);
                    rtl_sext(&s0, &s0, 1);
                    rtl_sr(R_AX, &s0, 1);
                }
                else
                {
                    rtl_sext(&reg_l(R_EAX), &reg_l(R_EAX), 2);
                }

                print_asm(decinfo.isa.is_operand_size_16 ? "cbtw" : "cwtl");
        }

make_EHelper(movsx)
        {
                id_dest->width = decinfo.isa.is_operand_size_16 ? 2 : 4;
        rtl_sext(&s0, &id_src->val, id_src->width);
        operand_write(id_dest, &s0);
        print_asm_template2(movsx);
        }

make_EHelper(movzx)
        {
                id_dest->width = decinfo.isa.is_operand_size_16 ? 2 : 4;
        operand_write(id_dest, &id_src->val);
        print_asm_template2(movzx);
        }
make_EHelper(movsb)
        {
                s0 = reg_l(R_ESI);
        rtl_lm(&s0, &s0, 1);
        rtl_sm(&reg_l(R_EDI), &s0, 1);
        reg_l(R_ESI) += 1;
        reg_l(R_EDI) += 1;

        print_asm_template2(movsb);
        }

make_EHelper(movsl)
        {
                s0 = reg_l(R_ESI);
        rtl_lm(&s0, &s0, 4);
        rtl_sm(&reg_l(R_EDI), &s0, 4);
        reg_l(R_ESI) += 4;
        reg_l(R_EDI) += 4;

        print_asm_template2(movsl);
        }
make_EHelper(lea)
        {
                operand_write(id_dest, &id_src->addr);
        print_asm_template2(lea);
        }
