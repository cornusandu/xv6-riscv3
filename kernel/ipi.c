#include "param.h"
#include "types.h"
#include "defs.h"

void
send_panic_ipi(void)
{
  for(int i = 0; i < NCPU; i++){
    if(i != cpuid()){
      *(uint32*)CLINT_MSI(i) = 1;
    }
  }
}
