#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

void main();
void timerinit();

// entry.S needs one stack per CPU.
__attribute__ ((aligned (16))) char stack0[4096 * NCPU];

// scratch area for timer interrupt, one per CPU.
uint64 mscratch0[NCPU * 32];

// assembly code in kernelvec.S for machine-mode timer interrupt.
extern void timervec();

// entry.S jumps here in machine mode on stack0.
void
start()
{
  // set M Previous Privilege mode to Supervisor, for mret.
  // the two-bit wide MPP holds the previous privilege mode up to machine mode
  unsigned long x = r_mstatus();
  x &= ~MSTATUS_MPP_MASK;
  // set the previous mode to supervisor
  // when call mret, the privilege mode is changed to supervisor
  x |= MSTATUS_MPP_S;
  w_mstatus(x);

  // set M Exception Program Counter to main, for mret.
  // requires gcc -mcmodel=medany
  w_mepc((uint64)main);

  // disable paging for now.
  // when the MODE field in satp is zero, there is no address translation 
  w_satp(0);

  // delegate all interrupts and exceptions to supervisor mode.
  // setting a bit in medeleg or mideleg will delegate the corresponding trap, when occurring in S-mode or U-mode, to the S-mode trap handler.
  // medeleg has a bit position allocated for every synchronous exception shown in the risc-v manual with the index of the bit position equal to the value in the mcause register
  // for local interrupt,(defined in SiFive) can not be delegated to supervisor mode
  w_medeleg(0xffff);
  // mideleg holds trap delegation bits for individual interrupts, with the layout of bits matching those in the mip register.
  w_mideleg(0xffff);
  
  // sie register contains  interrupt enable bits.
  // interrupt cause number i corresponds with bit i in sie
  // enable the supervisor-level external interrupt, supervisor-level timer interrupt and supervisor-level software interrupt
  w_sie(r_sie() | SIE_SEIE | SIE_STIE | SIE_SSIE);

  // ask for clock interrupts.
  timerinit();

  // keep each CPU's hartid in its tp register, for cpuid().
  int id = r_mhartid();
  // tp is thread pointer 
  w_tp(id);

  // switch to supervisor mode and jump to main().
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
  // A timer interrupt is pending whenever mtime is greater than or equal to the value in MTIMECMP register
  *(uint64*)CLINT_MTIMECMP(id) = *(uint64*)CLINT_MTIME + interval;

  // prepare information in scratch[] for timervec.
  // scratch[0..3] : space for timervec to save registers.
  // scratch[4] : address of CLINT MTIMECMP register.
  // scratch[5] : desired interval (in cycles) between timer interrupts.
  uint64 *scratch = &mscratch0[32 * id];
  scratch[4] = CLINT_MTIMECMP(id);
  scratch[5] = interval;
  // mscratch register is used by machine mode
  // used to hold a pointer to a machine-mode hart-local context space
  w_mscratch((uint64)scratch); 

  // set the machine-mode trap handler.
  // all traps into machine mode cause the pc to be set to the address of timervec
  w_mtvec((uint64)timervec);

  // enable machine-mode interrupts.
  // when a hart is executing in privilege mode x, interrupts are globally enabled when xIE=1 and globally disabled when xIE=0
  // interrupts for lower-privilege modes, w<x, are always globally disabled regardless of the setting of any global wIE bit for the lower-privilege mode
  w_mstatus(r_mstatus() | MSTATUS_MIE);

  // enable machine-mode timer interrupts.
  w_mie(r_mie() | MIE_MTIE);
}
