#define TIMEIO 0
#define TIMEFS 1
#define TIMEMM 2

#define THROUGHPUT 0

// TODO add locks
struct timemetric {
  uint64 total;
  uint64 min;
  uint64 max;
  uint num;
};

// TODO add locks
struct tpmetric {
	uint tick_count;
	uint exited_procs;
	uint min, max;
	uint total_exited_procs;
};
