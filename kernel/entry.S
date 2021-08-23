	# qemu -kernel loads the kernel at 0x80000000
        # and causes each CPU to jump there.
        # kernel.ld causes the following code to
        # be placed at 0x80000000.
        # Assembler treats all undefined symbols as external.
.section .text
_entry:
	# set up a stack for C.
        # stack0 is declared in start.c,
        # with a 4096-byte stack per CPU.
        # sp = stack0 + (hartid * 4096)
        # put the address of the symbol stack0 to the register sp
        la sp, stack0
        li a0, 1024*4
        # read the value of a CSR mhartid to register a1
        # mhartid is a read-only register containing the ID of a core(hardware thread)
        # the sp for every core points to the top of the stack
	csrr a1, mhartid
        addi a1, a1, 1
        mul a0, a0, a1
        add sp, sp, a0
	# jump to start() in start.c
        call start
        # spin
spin:
        j spin
