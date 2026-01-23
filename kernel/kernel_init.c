#include "defs.h"
#include "types.h"
#include "riscv.h"
#include "param.h"
#include "spinlock.h"
#include "kernel_init.h"
#include "proc.h"

extern struct proc proc[NPROC];

extern void kernel_reaper_main(void);

static uint64 child_pids[NPROC];

struct spinlock access_lock;

uint64*
get_child_pids(void)
{
  uint64* base = (uint64*)kalloc();
  struct proc *p = myproc();
  uint64 va = PGROUNDUP(p->sz);
  p->sz = va + PGSIZE;
  if (mappages(p->pagetable, va, PGSIZE, (uint64)base, PTE_V | PTE_U | PTE_R) < 0) {
    kfree(base);
    p->sz = va;
    return 0x0;
  }
  
  acquire(&access_lock);
  uint64 base_i = 0;
  for (uint64 i = 0; i < NPROC; i++) {
    if (child_pids[i] != 0) {
      base[base_i] = child_pids[i];
      base_i++;
    }
  }
  release(&access_lock);

  return (uint64*)va;
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
  initlock(&access_lock, "access_lock");

  struct proc *reaper;
  reaper = allocproc();

  acquire(&reaper->lock);

  reaper->cwd = namei("/");
  reaper->context.ra = (uint64)kernel_reaper_main;
  reaper->context.sp = reaper->kstack + PGSIZE;
  reaper->state = RUNNABLE;

  release(&reaper->lock);

  kernel_init_proc = reaper;
}

void
kernel_reaper_main(void)
{

  uint64 my_pid = myproc()->pid;

  for(;;){
    int result = kwait(0);
    if (result == -1) {
      ksleep(10);
    }

    acquire(&access_lock);
    for (uint64 i = 0; i < NPROC; i++) {
      acquire(&proc[i].lock);
      if (proc[i].parent == 0x0) {release(&proc[i].lock); continue;};
      if (proc[i].parent->pid == my_pid) {
        child_pids[i] = proc[i].pid;
      } else {
        child_pids[i] = 0;
      }
      release(&proc[i].lock);
    }
    release(&access_lock);
  }
}
