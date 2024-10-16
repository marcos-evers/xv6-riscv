#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "metrics.h"

struct timemetric tmtable[] = {
	[TIMEIO]  {0, 0, 0 ,0},
	[TIMEFS]  {0, 0, 0, 0},
	[TIMEMM]  {0, 0, 0, 0}
};

struct tpmetric tptable[] = {
	[THROUGHPUT] {0, 0, 0, 0, 0, 0}
};

static uint64
gettime()
{
  return r_time();
}

uint64
metrics_start(void)
{
  return gettime();
}

void
metrics_end(uint t, uint64 start)
{
  uint64 end = gettime();
  uint64 tot = end - start;
  struct timemetric* tm = &tmtable[t];
  tm->num++;
  tm->total += tot;
  if (tm->num == 1 || tot < tm->min)
    tm->min = tot;
  if (tm->num == 1 || tot > tm->max)
    tm->max = tot;
}

uint64
metrics_gettimenorm(uint t) {
  struct timemetric* tm = &tmtable[t];
  printf("%ld %ld %ld %d\n", tm->total, tm->min, tm->max, tm->num);
  uint64 norm = (tm->num * tm->max - tm->total) / (tm->num * (tm->max - tm->min));
  return norm;
}

struct tpmetric *
get_tpmetric() {
	return &tptable[THROUGHPUT];
}

int
incr_tick() {
	struct tpmetric *tpmetric = get_tpmetric();

	++tpmetric->tick_count;

	if(tpmetric->tick_count >= 10) {
		tpmetric->tick_count = 0;
		return 1;
	}

	return 0;
}

void
sec_update() {
	struct tpmetric *tpmetric = get_tpmetric();

	uint64 throughput = tpmetric->sec_exited_procs;
	if (tpmetric->total_sec == 1) {
		tpmetric->min = throughput;
		tpmetric->max = throughput;
	}
	else if (throughput < tpmetric->min) {
		tpmetric->min = throughput;
	}
	else if (throughput > tpmetric->max) {
		tpmetric->max = throughput;
	}

	++tpmetric->total_sec;
	tpmetric->total_exited_procs += throughput;
	tpmetric->sec_exited_procs = 0;
}

void
incr_exited_procs() {
	++get_tpmetric()->sec_exited_procs;
}
