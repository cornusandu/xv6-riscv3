#include <stdarg.h>

#include "types.h"
#include "defs.h"
#include "riscv.h"

extern int vprintf(char *fmt, va_list ap);

void oops(char* fmt, ...) {
    va_list ap;
    va_begin(ap);
    printf("oops: ");
    vprintf(fmt, ap);
    va_end(ap);

    uint64 scause = r_scause();
    uint64 stval = r_stval();

    if (myproc() == 0)
        panic("oops: triggered kernel oops from outside schedulable context\n");

    if (scause == 5)
        panic("oops: load access error (treated fatal)\n");
    else if (scause == 7)
        panic("oops: store access error (treated fatal)\n");
    else if (scause == 19)
        panic("oops: hardware error");

    if (scause == 18) {
        printf("oops: encountered software check; scause==18 (stval=0x%lx)\n", stval);
        kexit(138);
    }

    kexit(1);
}
