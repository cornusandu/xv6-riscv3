// Mutual exclusion lock.
struct spinlock {
  uint locked;       // Is the lock held?
  uint cpu_invalid;  // How many times has a kernel oops occured on the CPU in question?

  // For debugging:
  char *name;        // Name of lock.
  struct cpu *cpu;   // The cpu holding the lock.
};

