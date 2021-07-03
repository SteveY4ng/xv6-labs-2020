#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

void main();
void timerinit();

// entry.S needs one stack per CPU.
// __attribute__属于编译器对语言的扩展，而不是编程语言的标准内容
// 此处此变量会被编译器分配到16字节对齐的地址上
__attribute__ ((aligned (16))) char stack0[4096 * NCPU];

// scratch area for timer interrupt, one per CPU.
uint64 mscratch0[NCPU * 32];

// assembly code in kernelvec.S for machine-mode timer interrupt.
extern void timervec();

// entry.S jumps here in machine mode on stack0.
// start()函数做一些只允许在machine mode进行的配置，
void
start()
{
  // set M Previous Privilege mode to Supervisor, for mret.
  // 在machine mode，能够对mstatus寄存器进行读写
  // 此处将mstatus的内容读取出来
  unsigned long x = r_mstatus();
  // 在mstatus中mpp有两位，表示当前在machine mode，过去的mode为mpp位所表示的，这里将mpp位设置位supervisor mode，假装是从supervisor mode 通过函数调用来到machine mode
  x &= ~MSTATUS_MPP_MASK;
  x |= MSTATUS_MPP_S;
  // 修改后写回
  w_mstatus(x);

  // set M Exception Program Counter to main, for mret.
  // requires gcc -mcmodel=medany
  // 当发生trap陷入进入machine mode时，mepc寄存器会被写入发生异常指令所在的虚拟地址
  // 此处假装从main地址处发生trap，将main函数的地址写入mepc寄存器，
  w_mepc((uint64)main);

  // disable paging for now.
  // satp控制着在supervisor mode下地址转换和保护
  // satp的mode位为0时，表示在supervisor mode下没有地址转换和保护
  w_satp(0);

  // delegate all interrupts and exceptions to supervisor mode.
  // 将异常和终端处理逻辑交给supervisor mode，即如果medeleg或者mideleg中对应的异常中断为被设置为1，则对应的异常或者中断处理会陷入到supervisor mode进行处理，无论异常或则终端是发生在s mode还是u mode
  w_medeleg(0xffff);
  w_mideleg(0xffff);
  // 写入sie寄存器，打开s mode下的外部中断、时钟中断以及软件中断
  w_sie(r_sie() | SIE_SEIE | SIE_STIE | SIE_SSIE);

  // ask for clock interrupts.
  timerinit();

  // keep each CPU's hartid in its tp register, for cpuid().
  int id = r_mhartid();
  // 将每个cpu的hartid写入对应cpu的tp寄存器中
  w_tp(id);

  // switch to supervisor mode and jump to main().
  // 内联汇编
  // 在每个mode下都有对应的陷入trap返回指令
  // mret指令从m mode下的trap返回进入 s mode，并将pc值更新为mepc中的值，
  asm volatile("mret");
}

// set up to receive timer interrupts in machine mode,
// which arrive at timervec in kernelvec.S,
// which turns them into software interrupts for
// devintr() in trap.c.
void
timerinit()
{
  // each CPU has a separate source of timer interrupts.
  int id = r_mhartid();

  // ask the CLINT for a timer interrupt.
  int interval = 1000000; // cycles; about 1/10th second in qemu.
  *(uint64*)CLINT_MTIMECMP(id) = *(uint64*)CLINT_MTIME + interval;

  // prepare information in scratch[] for timervec.
  // scratch[0..3] : space for timervec to save registers.
  // scratch[4] : address of CLINT MTIMECMP register.
  // scratch[5] : desired interval (in cycles) between timer interrupts.
  uint64 *scratch = &mscratch0[32 * id];
  scratch[4] = CLINT_MTIMECMP(id);
  scratch[5] = interval;
  w_mscratch((uint64)scratch);

  // set the machine-mode trap handler.
  w_mtvec((uint64)timervec);

  // enable machine-mode interrupts.
  w_mstatus(r_mstatus() | MSTATUS_MIE);

  // enable machine-mode timer interrupts.
  w_mie(r_mie() | MIE_MTIE);
}
