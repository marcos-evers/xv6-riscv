#define TIMEIO 0
#define TIMEFS 1
#define TIMEMM 2

#define THROUGHPUT 0

struct timemetric {
  uint64 total;
  uint64 min;
  uint64 max;
  uint num;
};

struct tpmetric {
	uint tick_count;
	uint exited_procs;
	uint min, max;
	uint total_exited_procs;
};
