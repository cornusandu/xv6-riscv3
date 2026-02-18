// tty.c -- an output-only, modernized version of console.c

#include "types.h"
#include "param.h"
#include "riscv.h"
#include "spinlock.h"
#include "defs.h"
#include "proc.h"
#include <stdalign.h>

// alignas(64) is used to make sure each variable has its own cache line,
//  to not slow down TTY output on many-core systems (ex. 64 cores)

static alignas(64) int64 lock_owner;
static alignas(64) volatile uint last_access;
static alignas(64) volatile uint8 writing;

static alignas(64) volatile uint8 tty_already_init = 0;

static alignas(64) struct spinlock global_lock;
static alignas(64) struct spinlock global_data_lock;

void
tty_init(void) {
  if (tty_already_init == 1) return;
  tty_already_init = 1;

  lock_owner = -1;
  last_access = 0;
  writing = 0;

  initlock(&global_lock, "tty_glbl_lock");
  initlock(&global_data_lock, "tty_glbl0_lock");
}

static inline void
spin_until_ticks(uint64 start, uint64 n)
{
  while(1){
    acquire(&tickslock);
    uint64 now = ticks;
    release(&tickslock);

    if(now - start >= n)
      break;

    // polite spin: allow interrupt handling
    asm volatile("nop");
  }
}

static inline void
uart_guard_delay(void)
{
  // Small, bounded, CPU-local delay.
  // Does NOT depend on interrupts, timers, or shared state.
  for (volatile int i = 0; i < 500; i++) {
    asm volatile("nop");
  }
}

void
tty_putc(char c)
{
  uint8 retries = 0;
  goto_start:

  acquire(&global_data_lock);
  if (lock_owner == -1 || lock_owner == (myproc() ? myproc()->pid : cpuid() + NPROC)) {
    lock_owner = myproc() ? myproc()->pid : cpuid() + NPROC;
    release(&global_data_lock);
  } else {
    release(&global_data_lock);

    uart_guard_delay();
    
    acquire(&global_data_lock);
    if (lock_owner != -1 && writing == 0 && retries < 5) {
      release(&global_data_lock);
      retries += 1;
      goto goto_start;
    }
    lock_owner = myproc() ? myproc()->pid : cpuid() + NPROC;
    release(&global_data_lock);
  }

  uint8 prev = intr_get();

  intr_off();

  acquire(&global_lock);

  acquire(&global_data_lock);
  writing = 1;
  release(&global_data_lock);

  if(c == '\b'){
    uartputc_sync('\b');
    uartputc_sync(' ');
    uartputc_sync('\b');
  } else {
    uartputc_sync(c);
  }

  //acquire(&tickslock);
  //last_access = ticks;
  //release(&tickslock);
  
  acquire(&global_data_lock);
  writing = 0;
  
  release(&global_lock);
  release(&global_data_lock);

  if (prev)
    intr_on();
}
