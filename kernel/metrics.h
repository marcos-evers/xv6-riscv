#define TIMEFS 0
#define TIMEMM 1

#define THROUGHPUT 0

#define FAIRNESS 0

struct timemetric {
  struct spinlock lock;

  uint64 total;
  uint num;
};

struct tpmetric {
  struct spinlock lock;

  uint ntick;
  uint nexited;
};

struct process
{
  int pid;
  uint64 time;
  uint64 start, end;
};

struct fairmetric
{
  struct spinlock lock;
  struct process procs[NPROC];
  uint n_proc;
};
