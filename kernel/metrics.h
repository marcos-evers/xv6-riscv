#define TIMEIO 0
#define TIMEFS 1
#define TIMEMM 2

#define THROUGHPUT 0

struct timemetric {
  uint64 avg;

  uint min;
  uint max;
  uint num;
};

struct tpmetric {
	uint tick_count;

	uint sec_exited_procs;

	uint min, max;
	uint total_sec;
	uint total_exited_procs;
};
