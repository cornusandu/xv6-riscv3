struct buf;
struct context;
struct file;
struct inode;
struct pipe;
struct proc;
struct spinlock;
struct sleeplock;
struct stat;
struct superblock;
struct procdata;
struct logheader;

// bio.c
extern  void            binit(void);
extern  struct buf*     bread(uint, uint);
extern  void            brelse(struct buf*);
extern  void            bwrite(struct buf*);
extern  void            bpin(struct buf*);
extern  void            bunpin(struct buf*);

// console.c
extern  void            consoleinit(void);
extern  void            consoleintr(int);
extern  void            consputc(int);

// exec.c
extern  int             kexec(char*, char**);

// file.c
extern  struct file*    filealloc(void);
extern  void            fileclose(struct file*);
extern  struct file*    filedup(struct file*);
extern  void            fileinit(void);
extern  int             fileread(struct file*, uint64, int n);
extern  int             filestat(struct file*, uint64 addr);
extern  int             filewrite(struct file*, uint64, int n);

// fs.c
extern  void            fsinit(int);
extern  int             dirlink(struct inode*, char*, uint);
extern  struct inode*   dirlookup(struct inode*, char*, uint*);
extern  struct inode*   ialloc(uint, short);
extern  struct inode*   idup(struct inode*);
extern  void            iinit();
extern  void            ilock(struct inode*);
extern  void            iput(struct inode*);
extern  void            iunlock(struct inode*);
extern  void            iunlockput(struct inode*);
extern  void            iupdate(struct inode*);
extern  int             namecmp(const char*, const char*);
extern  struct inode*   namei(char*);
extern  struct inode*   nameiparent(char*, char*);
extern  int             readi(struct inode*, int, uint64, uint, uint);
extern  void            stati(struct inode*, struct stat*);
extern  int             writei(struct inode*, int, uint64, uint, uint);
extern  void            itrunc(struct inode*);
extern  void            ireclaim(int);

// kalloc.c
extern  void*           kalloc(void);
extern  void            kfree(void *);
extern  void            kinit(void);

// log.c
extern  void            initlog(int, struct superblock*);
extern  void            log_write(struct buf*);
extern  void            begin_op(void);
extern  void            end_op(void);

// pipe.c
extern  int             pipealloc(struct file**, struct file**);
extern  void            pipeclose(struct pipe*, int);
extern  int             piperead(struct pipe*, uint64, int);
extern  int             pipewrite(struct pipe*, uint64, int);

// printf.c
extern  int             printf(char*, ...) __attribute__ ((format (printf, 1, 2)));
extern  void            panic(char*) __attribute__((noreturn));
extern  void            printfinit(void);
extern  void            block_if_panic(void);

// proc.c
extern  int               cpuid(void);
extern  void              kexit(int);
extern  int               kfork(void);
extern  int               growproc(int);
extern  void              proc_mapstacks(pagetable_t);
extern  pagetable_t       proc_pagetable(struct proc *);
extern  void              proc_freepagetable(pagetable_t, uint64);
extern  int               kkill(int);
extern  int               killed(struct proc*);
extern  void              setkilled(struct proc*);
extern  struct cpu*       mycpu(void);
extern  struct proc*      myproc();
extern  void              procinit(void);
extern  void              scheduler(void) __attribute__((noreturn));
extern  void              sched(void);
extern  void              sleep(void*, struct spinlock*);
extern  void              userinit(void);
extern  int               kwait(uint64);
extern  void              wakeup(void*);
extern  void              yield(void);
extern  int               either_copyout(int user_dst, uint64 dst, void *src, uint64 len);
extern  int               either_copyin(void *dst, int user_src, uint64 src, uint64 len);
extern  void              procdump(void);
extern  struct proc*      proc_create(void);
extern  void              kfree_proc(struct proc *);
extern  void              kps(void);
extern  struct procdata   pinfo(uint64);

// swtch.S
extern  void            swtch(struct context*, struct context*);

// spinlock.c
extern  void            acquire(struct spinlock*);
extern  int             holding(struct spinlock*);
extern  void            initlock(struct spinlock*, char*);
extern  void            release(struct spinlock*);
extern  void            push_off(void);
extern  void            pop_off(void);

// sleeplock.c
extern  void            acquiresleep(struct sleeplock*);
extern  void            releasesleep(struct sleeplock*);
extern  int             holdingsleep(struct sleeplock*);
extern  void            initsleeplock(struct sleeplock*, char*);

// string.c
extern  int             memcmp(const void*, const void*, uint);
extern  void*           memmove(void*, const void*, uint);
extern  void*           memset(void*, int, uint);
extern  char*           safestrcpy(char*, const char*, int);
extern  int             strlen(const char*);
extern  int             strncmp(const char*, const char*, uint);
extern  char*           strncpy(char*, const char*, int);

// syscall.c
extern  void            argint(int, int*);
extern  int             argstr(int, char*, int);
extern  void            argaddr(int, uint64 *);
extern  int             fetchstr(uint64, char*, int);
extern  int             fetchaddr(uint64, uint64*);
extern  void            syscall();

// trap.c
extern  uint   ticks;
extern  void            trapinit(void);
extern  void            trapinithart(void);
extern  struct spinlock tickslock;
extern  void            prepare_return(void);

// uart.c
extern  void            uartinit(void);
extern  void            uartintr(void);
extern  void            uartwrite(char [], int);
extern  void            uartputc_sync(int);
extern  int             uartgetc(void);

// vm.c
extern  void            kvminit(void);
extern  void            kvminithart(void);
extern  void            kvmmap(pagetable_t, uint64, uint64, uint64, int);
extern  int             mappages(pagetable_t, uint64, uint64, uint64, int);
extern  pagetable_t     uvmcreate(void);
extern  uint64          uvmalloc(pagetable_t, uint64, uint64, int);
extern  uint64          uvmdealloc(pagetable_t, uint64, uint64);
extern  int             uvmcopy(pagetable_t, pagetable_t, uint64);
extern  void            uvmfree(pagetable_t, uint64);
extern  void            uvmunmap(pagetable_t, uint64, uint64, int);
extern  void            uvmclear(pagetable_t, uint64);
extern  pte_t *         walk(pagetable_t, uint64, int);
extern  uint64          walkaddr(pagetable_t, uint64);
extern  int             copyout(pagetable_t, uint64, char *, uint64);
extern  int             copyin(pagetable_t, char *, uint64, uint64);
extern  int             copyinstr(pagetable_t, char *, uint64, uint64);
extern  int             ismapped(pagetable_t, uint64);
extern  uint64          vmfault(pagetable_t, uint64, int);

// plic.c
extern  void            plicinit(void);
extern  void            plicinithart(void);
extern  int             plic_claim(void);
extern  void            plic_complete(int);

// virtio_disk.c
extern  void            virtio_disk_init(void);
extern  void            virtio_disk_rw(struct buf *, int);
extern  void            virtio_disk_intr(void);

// kasserts.c
extern  void            run_asserts(void);
extern  void            late_asserts(void);

// tty.c
extern  void            tty_init(void);
extern  void            tty_putc(char);

// number of elements in fixed-size array
#define NELEM(x) (sizeof(x)/sizeof((x)[0]))
