#define TIMEFS 0
#define TIMEMM 1

#define FAIRNESS 0

struct timemetric {
  struct spinlock lock;
  uint64 total;
};

struct process
{
  int pid;
  uint64 time;
  uint64 start;
};

struct fairmetric
{
  struct spinlock lock;
  struct process procs[NPROC];
  uint n_proc;
};
