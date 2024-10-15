#define TIMEIO 0
#define TIMEFS 1
#define TIMEMM 2

#define THROUGHPUTPROC 0

struct timemetric {
  uint64 avg;

  uint min;
  uint max;
  uint num;
};
