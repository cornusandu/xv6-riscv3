#include "param.h"
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "spinlock.h"
#include "proc.h"
#include "stddef.h"
#include "log.h"
#include "fs.h"

#define ALIGNOF(type) offsetof(struct { char c; type t; }, t)

static void
ASSERT(unsigned char value, const char* const msg)
{
  printf("kasserts >> ASSERT() >> Running condition: %s", msg);
  if (value != 1) {
    printf("panic: kasserts >> ASSERT() >> Condition failed: %s", msg);
    panic("kasserts >> ASSERT(false)");
  }
}

void
run_asserts(void)
{
  ASSERT(sizeof(void*) == 8, "Machine is 64-bit\n");
  ASSERT(ALIGNOF(enum procstate) <= ALIGNOF(int), "Alignment of (enum procstate) is lower or equal to alignment of int.\n");
  ASSERT(sizeof(struct logheader) < BSIZE, "Size of logheader is within bounds\n");
}
