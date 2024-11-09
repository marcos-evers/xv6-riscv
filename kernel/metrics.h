#define TIMEIO 0
#define TIMEFS 1
#define TIMEMM 2

#define THROUGHPUT 0

#define FAIRNESS 0

struct timemetric {
  struct spinlock lock;

  uint64 total;
  uint64 min;
  uint64 max;
  uint num;
};

struct tpmetric {
  struct spinlock lock;

  uint tick_count;
  uint exited_procs;
	uint min, max;
	uint total_exited_procs;
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
