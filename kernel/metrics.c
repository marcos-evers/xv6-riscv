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
	[THROUGHPUT] {0, 0, 0, 0, 0}
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

void
metrics_tick()
{
	struct tpmetric *tpmetric = &tptable[THROUGHPUT];
  uint tp = tpmetric->exited_procs;

	++tpmetric->tick_count;
  tpmetric->total_exited_procs += tp;
  tpmetric->exited_procs = 0;

	if (tpmetric->tick_count == 1 && tp < tpmetric->min) {
		tpmetric->min = tp;
	} else if (tpmetric->tick_count == 1 && tp > tpmetric->max) {
		tpmetric->max = tp;
	}
}

void
metrics_proc_exited()
{
	++tptable[THROUGHPUT].exited_procs;
}
