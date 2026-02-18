#include "types.h"
#include "param.h"
#include "riscv.h"
#include "defs.h"
#include "spinlock.h"
#include "kernel_init.h"

volatile static int started = 0;
int hart_started[NPROC];

void
init_early(void)
{
  consoleinit();
  tty_init();
  printfinit();
  memset((void*)hart_started, 0, NPROC * sizeof(int));
  printf("\n");
  printf("init_early() called\nEntered early state.\n");
  run_asserts();
}

void
init_hardware(void)
{
  printf("init_hardware: Initialise RAM\n");

  kinit();         // physical page allocator
  kvminit();       // create kernel page table
  kvminithart();   // turn on paging

  printf("init_hardware: Initialise processes\n");

  procinit();      // process table

  printf("init_hardware: Initialise interrupts\n");

  trapinit();      // trap vectors
  trapinithart();  // install kernel trap vector
  plicinit();      // set up interrupt controller
  plicinithart();  // ask PLIC for device interrupts

  printf("init_hardware: Initialise file system\n");

  binit();         // buffer cache
  iinit();         // inode table
  fileinit();      // file table
  printf("\n");
}

// start() jumps here in supervisor mode on all CPUs.
void
main()
{
  if(cpuid() == 0){
    init_early();
    printf("xv6 kernel is booting\n");
    init_hardware();
    printf("initialized hardware state\n");
    virtio_disk_init(); // emulated hard disk
    printf("initialised emulated hard disk\n");
    hart_started[0] = 1;

    late_asserts();

    printf("entering userinit()\n");
    userinit();      // first user process
    printf("exiting userinit()\n");
    kernel_init_create();
    __sync_synchronize();
    started = 1;
  } else {
    while(started == 0)
      ;
    __sync_synchronize();
    printf("hart %d starting\n", cpuid());
    kvminithart();    // turn on paging
    trapinithart();   // install kernel trap vector
    plicinithart();   // ask PLIC for device interrupts
    hart_started[cpuid()] = 1;
    late_asserts();
  }

  scheduler();        
}
