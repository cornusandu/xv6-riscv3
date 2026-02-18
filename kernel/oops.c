#include <stdarg.h>

#include "types.h"
#include "param.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

static uint64 total_oops = 0;
static uint16 semi_fatal_oops = 0;

extern int vprintf(char *fmt, va_list ap);

void oops(char* fmt, ...) {
    mystate()->ninvalid++;

    va_list ap;
    va_start(ap, fmt);
    printf("oops: ");
    vprintf(fmt, ap);
    va_end(ap);

    uint64 scause = r_scause();
    uint64 stval = r_stval();

    if (myproc() == 0)
        panic("oops: triggered kernel oops outside a schedulable context\n");

    if (mystate()->intrap == INTRAP_0)
        kexit(1);

    if (mystate()->intrap == INTRAP_U)
        panic("oops: user mode triggered kernel oops (invalid control flow)\n");

    if (scause == 5)
        panic("oops: load access error (treated fatal)\n");
    else if (scause == 7)
        panic("oops: store access error (treated fatal)\n");
    else if (scause == 19)
        panic("oops: hardware error");

    if (scause == 18) {
        printf("oops: encountered software check; scause==18 (stval=0x%lx)\n", stval);
        mystate()->intrap = INTRAP_0;
        kexit(138);
    }

    if (scause == 0)
        panic("oops: instruction address misaligned (treated fatal)\n");
    else if (scause == 1)
        panic("oops: instruction access fault (treated fatal)\n");
    else if (scause == 12)
        panic("oops: instruction page fault (treated fatal)\n");
    
    if (scause == 2)
        printf("oops: illegal instruction\n");
    else if (scause==4)
        printf("oops: load adress misaligned\n");
    else if (scause==6) {
        printf("oops: store address misaligned\n");
        semi_fatal_oops++;
    }
    else if (scause==13)
        printf("oops: load page fault\n");
    else if (scause==15) {
        printf("oops: store page fault\n");
        semi_fatal_oops++;
    }
    else
        printf("oops: unknown trap reason\n");

    total_oops++;

    if (total_oops >= 200)
        panic("oops: too many oops\n");
    else if (semi_fatal_oops >= 10)
        panic("oops: too many dangerous oops\n");
    
    mystate()->intrap = INTRAP_0;
    kexit(1);
}
