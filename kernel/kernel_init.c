#include "types.h"
#include "riscv.h"
#include "param.h"
#include "spinlock.h"
#include "defs.h"
#include "kernel_init.h"
#include "proc.h"

extern struct proc proc[NPROC];

extern struct spinlock wait_lock;

extern void kernel_reaper_main(void);

struct proc *kernel_init_proc = 0x0;


int
kwait_nonblocking(uint64 addr)
{
  struct proc *pp;
  struct proc *p = myproc();
  int havekids = 0;
  int pid = 0;

  acquire(&wait_lock);

  for (pp = proc; pp < &proc[NPROC]; pp++) {
    if (pp->parent == p) {
      havekids = 1;

      acquire(&pp->lock);
      if (pp->state == ZOMBIE) {
        pid = pp->pid;
        if (addr != 0 &&
            copyout(p->pagetable, addr,
                    (char *)&pp->xstate,
                    sizeof(pp->xstate)) < 0) {
          release(&pp->lock);
          release(&wait_lock);
          return -1;
        }
        kfree_proc(pp);
        release(&pp->lock);
        release(&wait_lock);
        return pid;
      }
      release(&pp->lock);
    }
  }

  release(&wait_lock);

  if (!havekids || killed(p))
    return -1;

  return 0;   // children exist, none exited yet
}

void
ksleep(int ticks_to_sleep)
{
  uint start;

  acquire(&tickslock);
  start = ticks;
  while(ticks - start < ticks_to_sleep){
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
}


void
kernel_init_create(void)
{
  printf("kernel_init_create: Initialising process\n");
  struct proc *reaper;
  reaper = proc_create();
  reaper->cwd = namei("/");

  reaper->trapframe->epc = (uint64)kernel_reaper_main;
  reaper->context.sp = reaper->kstack + PGSIZE;
  reaper->state = RUNNABLE;
  reaper->intended_state = INTENDED_S;
  memmove(&reaper->name, (void*)"kreap", 6);
  
  printf("kernel_init_create: Preparing kernel execution context\n");
  extern void kernelvec(void);
  w_stvec((uint64)kernelvec);

  uint64 s = r_sstatus();
  s |= SSTATUS_SPP;   // set bit 8
  w_sstatus(s);

  release(&reaper->lock);

  kernel_init_proc = reaper;

  printf("kernel_init_create: Process scheduled for execution\n");
}

void
kernel_reaper_main(void)
{

  printf("Starting kernel_reaper\n");

  //uint64 my_pid = myproc()->pid;

  for(;;){
    int result = kwait_nonblocking(0);
    //*(char*)0=0;   // if you want to test out the effects of a kernel oops
    if (result == -1) {
      //ksleep(10);
    }
  }
}
