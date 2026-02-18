// Sleeping locks

#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sleeplock.h"

extern struct proc proc[NPROC];

void
initsleeplock(struct sleeplock *lk, char *name)
{
  initlock(&lk->lk, "sleep lock");
  lk->name = name;
  lk->locked = 0;
  lk->pid = 0;
}

void
acquiresleep(struct sleeplock *lk)
{
  acquire(&lk->lk);
  while (lk->locked) {
    // Ensure that the process that previously owned the sleep lock
    // didn't get terminated by a kernel oops

    uint owner_still_exists = 0;
    for (uint32 i = 0; i < NPROC; i++) {
      if (proc[i].pid == lk->pid) {
        acquire(&proc[i].lock);
        if (proc[i].state == RUNNABLE || proc[i].state == RUNNING || proc[i].state == SLEEPING) {
          owner_still_exists = 1;
          release(&proc[i].lock);
          break;
        } else {
          release(&proc[i].lock);
          break;
        }
        release(&proc[i].lock);
      }
    }

    // If the previous owner was terminated by a kernel oops,
    // the lock is invalidated and can be reacquired by another
    // process  (THIS IS NOT GUARANTEED TO BE SAFE, AND IS A
    // TRIVIAL ATTEMPT AT RECOVERY AND FAULT CONTAINMENT)
    if (owner_still_exists == 0)
      break;
    
    sleep(lk, &lk->lk);
  }
  lk->locked = 1;
  lk->pid = myproc()->pid;
  release(&lk->lk);
}

void
releasesleep(struct sleeplock *lk)
{
  acquire(&lk->lk);
  lk->locked = 0;
  lk->pid = 0;
  wakeup(lk);
  release(&lk->lk);
}

int
holdingsleep(struct sleeplock *lk)
{
  int r;
  
  acquire(&lk->lk);
  r = lk->locked && (lk->pid == myproc()->pid);
  release(&lk->lk);
  return r;
}



