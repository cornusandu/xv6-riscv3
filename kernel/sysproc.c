#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"

extern struct proc proc[NPROC];

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64 sys_pinfo(void)
{
  struct proc *p = myproc();
  if (!p) {return -1;}

  int pid;
  argint(0, &pid);
  uint64 result;
  argaddr(1, &result);

  struct procdata data = pinfo(pid);
  
  if (p->pid == 1)                     goto goto_skip_checks;
  if (p->intended_state == INTENDED_S) goto goto_skip_checks;
  if (pid == p->pid)                   goto goto_skip_checks;

  if (data.parent_pid != p->pid) {
    data.xstate = 0;
    if (data.state == RUNNING) data.state = RUNNABLE;
    if (data.state == USED)    data.state = RUNNABLE;
    if (data.state == UNUSED)  data.state = ZOMBIE;
  }

  goto_skip_checks:

  if (copyout(p->pagetable, result, (char*)&data, sizeof(data)) < 0) {
    return -1;
  };

  return 0;
}

uint64 sys_ps(void)
{
  struct proc *p = myproc();
  if (!p) {return -1;}

  uint64 result;
  argaddr(0, &result);
  uint64 upper_bound;  //  max output lenght in bytes
  argaddr(1, &upper_bound);
  if (upper_bound == 0) {upper_bound--;}

  uint64 counter = 0;

  for (uint64 i = 0; i < NPROC; i++) {
    struct proc *process = &proc[i];

    acquire(&process->lock);

    if (process->pid > 0 && \
        process->state != ZOMBIE && \
        process->state != UNUSED) {

      if (copyout(p->pagetable, result + counter, (char*)&process->pid, sizeof(process->pid)) < 0) {
        release(&process->lock);
        return -1;
      };
      counter += sizeof(process->pid);
      if (counter + sizeof(int) > upper_bound * sizeof(int))
        {release(&process->lock); return 1;};
    }

    release(&process->lock);
  }

  return 0;
};

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if(t == SBRK_EAGER || n < 0) {
    if(growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if(addr + n < addr)
      return -1;
    if(addr + n > TRAPFRAME)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
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
