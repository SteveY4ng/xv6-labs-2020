#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if (argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if (argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if (argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if (argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n)
  {
    if (myproc()->killed)
    {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  backtrace();
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if (argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_sigreturn(void)
{
  struct proc *p = myproc();
  p->trapframe->epc = p->alarmframe->epc;
  p->trapframe->ra = p->alarmframe->ra;
  p->trapframe->sp = p->alarmframe->sp;
  p->trapframe->gp = p->alarmframe->gp;
  p->trapframe->tp = p->alarmframe->tp;
  p->trapframe->t0 = p->alarmframe->t0;
  p->trapframe->t1 = p->alarmframe->t1;
  p->trapframe->t2 = p->alarmframe->t2;
  p->trapframe->s0 = p->alarmframe->s0;
  p->trapframe->s1 = p->alarmframe->s1;
  p->trapframe->a0 = p->alarmframe->a0;
  p->trapframe->a1 = p->alarmframe->a1;
  p->trapframe->a2 = p->alarmframe->a2;
  p->trapframe->a3 = p->alarmframe->a3;
  p->trapframe->a4 = p->alarmframe->a4;
  p->trapframe->a5 = p->alarmframe->a5;
  p->trapframe->a6 = p->alarmframe->a6;
  p->trapframe->a7 = p->alarmframe->a7;
  p->trapframe->s2 = p->alarmframe->s2;
  p->trapframe->s3 = p->alarmframe->s3;
  p->trapframe->s4 = p->alarmframe->s4;
  p->trapframe->s5 = p->alarmframe->s5;
  p->trapframe->s6 = p->alarmframe->s6;
  p->trapframe->s7 = p->alarmframe->s7;
  p->trapframe->s8 = p->alarmframe->s8;
  p->trapframe->s9 = p->alarmframe->s9;
  p->trapframe->s10 = p->alarmframe->s10;
  p->trapframe->s11 = p->alarmframe->s11;
  p->trapframe->t3 = p->alarmframe->t3;
  p->trapframe->t4 = p->alarmframe->t4;
  p->trapframe->t5 = p->alarmframe->t5;
  p->trapframe->t6 = p->alarmframe->t6;
  kfree((void*)p->alarmframe);
  p->alarmframe=0;
  return 0;
}

uint64
sys_sigalarm(void)
{
  int ticks;
  uint64 handler;
  // Get the first argument
  if (argint(0, &ticks) < 0)
    return -1;

  // Get the second argument
  if (argaddr(1, &handler) < 0)
    return -1;

  myproc()->alarm_interval = ticks;
  myproc()->alarm_handler = handler;
  myproc()->passed_ticks =0;

  return 0;
}
