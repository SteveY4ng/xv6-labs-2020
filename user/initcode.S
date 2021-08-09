# Initial process that execs /init.
# This code runs in user space.

#include "syscall.h"
# 将参数放在寄存器a0和寄存器a1中，将系统调用号存放在寄存器a7中，ecall指令陷入内核
# exec(init, argv)
.globl start
start:
        la a0, init
        la a1, argv
        li a7, SYS_exec
        ecall

# for(;;) exit();
exit:
        li a7, SYS_exit
        ecall
        jal exit

# char init[] = "/init\0";
init:
  .string "/init\0"

# char *argv[] = { init, 0 };
.p2align 2
argv:
  .long init
  .long 0
