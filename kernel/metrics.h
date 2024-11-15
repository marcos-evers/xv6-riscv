#define TIMEFS 0
#define TIMEMM 1

#define FAIRNESS 0

struct timemetric {
  struct spinlock lock;
  uint64 total;
  uint64 num;
};

struct process
{
  int pid;
  uint64 time;
  uint64 start;
  uint sched;
};

struct fairmetric
{
  struct spinlock lock;
  struct process procs[NPROC];
  uint n_proc;
};
