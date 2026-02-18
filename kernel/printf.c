//
// formatted console output -- printf, panic.
//

#include <stdarg.h>

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"

volatile int panicking = 0; // printing a panic message
volatile int panicked = 0; // spinning forever at end of a panic

// lock to avoid interleaving concurrent printf's.
static struct {
  struct spinlock lock;
} pr;

static char digits[] = "0123456789abcdef";

static void
printint(long long xx, int base, int sign)
{
  char buf[20];
  int i;
  unsigned long long x;

  if(sign && (sign = (xx < 0)))
    x = -xx;
  else
    x = xx;

  i = 0;
  do {
    buf[i++] = digits[x % base];
  } while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  while(--i >= 0)
    tty_putc(buf[i]);
}

static void
printptr(uint64 x)
{
  int i;
  tty_putc('0');
  tty_putc('x');
  for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
    tty_putc(digits[x >> (sizeof(uint64) * 8 - 4)]);
}

// Print to the console.
int
vprintf(char *fmt, va_list ap)
{
  int i, cx, c0, c1, c2;
  char *s;

  block_if_panic();

  if(panicking == 0)
    acquire(&pr.lock);

  for(i = 0; (cx = fmt[i] & 0xff) != 0; i++){
    if(cx != '%'){
      tty_putc(cx);
      continue;
    }
    i++;
    c0 = fmt[i+0] & 0xff;
    c1 = c2 = 0;
    if(c0) c1 = fmt[i+1] & 0xff;
    if(c1) c2 = fmt[i+2] & 0xff;
    if(c0 == 'd'){
      printint(va_arg(ap, int), 10, 1);
    } else if(c0 == 'l' && c1 == 'd'){
      printint(va_arg(ap, uint64), 10, 1);
      i += 1;
    } else if(c0 == 'l' && c1 == 'l' && c2 == 'd'){
      printint(va_arg(ap, uint64), 10, 1);
      i += 2;
    } else if(c0 == 'u'){
      printint(va_arg(ap, uint32), 10, 0);
    } else if(c0 == 'l' && c1 == 'u'){
      printint(va_arg(ap, uint64), 10, 0);
      i += 1;
    } else if(c0 == 'l' && c1 == 'l' && c2 == 'u'){
      printint(va_arg(ap, uint64), 10, 0);
      i += 2;
    } else if(c0 == 'x'){
      printint(va_arg(ap, uint32), 16, 0);
    } else if(c0 == 'l' && c1 == 'x'){
      printint(va_arg(ap, uint64), 16, 0);
      i += 1;
    } else if(c0 == 'l' && c1 == 'l' && c2 == 'x'){
      printint(va_arg(ap, uint64), 16, 0);
      i += 2;
    } else if(c0 == 'p'){
      printptr(va_arg(ap, uint64));
    } else if(c0 == 'c'){
      tty_putc(va_arg(ap, uint));
    } else if(c0 == 's'){
      if((s = va_arg(ap, char*)) == 0)
        s = "(null)";
      for(; *s; s++)
        tty_putc(*s);
    } else if(c0 == '%'){
      tty_putc('%');
    } else if(c0 == 0){
      break;
    } else {
      // Print unknown % sequence to draw attention.
      tty_putc('%');
      tty_putc(c0);
    }

  }

  if(panicking == 0)
    release(&pr.lock);

  return 0;
}

int printf(char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int r = vprintf(fmt, ap);
  va_end(ap);
  return r;
}

[[noreturn]] static void
block(void)
{
  mystate()->intrap = 0xFFFF;
  intr_off();
  __sync_synchronize();
  for(;;)
    ;
}

void
panic(char *s)
{
  mystate()->intrap = 0xFFFF;
  __sync_synchronize();
  panicking = 1;
  __sync_synchronize();
  printf("panic: ");
  __sync_synchronize();
  printf("%s\n", s);
  __sync_synchronize();
  panicked = 1; // freeze uart output from other CPUs

  block();
}

void
block_if_panic(void)
{
  if (panicked) {
    block();
  }
}

void
printfinit(void)
{
  initlock(&pr.lock, "pr");
}
