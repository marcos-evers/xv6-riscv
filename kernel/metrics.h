#define TIMEIO 0
#define TIMEFS 1
#define TIMEMM 2

#define THROUGHPUT 0

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
