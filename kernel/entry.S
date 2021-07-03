	# qemu -kernel loads the kernel at 0x80000000
        # and causes each CPU to jump there.
        # kernel.ld causes the following code to
        # be placed at 0x80000000.
        # 汇编中以.开头的名称用来给汇编器一些特殊指示，称为汇编指示
        # .section 指示把代码划分为若干个段，程序被操作系统加载执行时，每个段被加载到不同的地址， 每个段可能会有不同的读、写、执行权限
        # .text 段保存代码，是可读以及可执行的
        # _entry 符号代表当前指令地址
.section .text
_entry:
	# set up a stack for C.
        # stack0 is declared in start.c,
        # with a 4096-byte stack per CPU.
        # sp = stack0 + (hartid * 4096)
        la sp, stack0
        li a0, 1024*4
        # 读取当前核的hartid到a1寄存器中
	csrr a1, mhartid
        addi a1, a1, 1
        mul a0, a0, a1
        add sp, sp, a0
	# jump to start() in start.c
        call start
spin:
        j spin
