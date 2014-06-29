#include "types.h"
#include "x86.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return proc->pid;
}

int
sys_sbrk(void)
{
  /* Old Implementaion
  int addr;
  int n;
  if(argint(0, &n) < 0)
    return -1;
  // cprintf("before call sys_sbrk, sz = %x\n", proc->sz);
  // cprintf("call sys_sbrk = %d\n", n);
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  proc->sz += n;
  return addr;
  */
  int addr;
  int n;
  if (argint(0, &n) < 0) {
    return -1;
  }

  uint sz = proc->sz;
  if (n < 0) {
    if ((sz = deallocuvm(proc->pgdir, sz, sz + n)) == 0) {
      return -1;
    }
    proc->sz = sz;
    addr = proc->sz;
  } else {
    addr = proc->sz;
    proc->sz += n;
  }
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int
sys_halt(void) {
  char *p = "Shutdown";
  for (; *p; p++) {
    outb(0x8900, *p);
  }
  return 0;
}

int
sys_alarm(void) {
  int ticks;
  void (*handler)();
  if (argint(0, &ticks) < 0) {
    return -1;
  }
  if (argptr(1, (char**)&handler, 1) < 0) {
    return -1;
  }
  proc->alarmticks = ticks;
  proc->alarmhandler = handler;
  return 0;
}

int
sys_handler_ret(void) {
  struct trapframe *tf = proc->tf;
  if (tf->esp >= KERNBASE) {
    return -1;
  }

  tf->esp = tf->esp + 8;
  tf->edx = *((uint*)tf->esp);

  tf->esp = tf->esp + 4;
  tf->ecx = *((uint*)tf->esp);

  tf->esp = tf->esp + 4;
  tf->eax = *((uint*)tf->esp);

  tf->esp = tf->esp + 4;
  tf->eip = *((uint*)tf->esp);

  tf->esp += 4;
  return 0;
}
