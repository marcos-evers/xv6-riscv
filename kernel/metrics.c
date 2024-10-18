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

void
metrics_reset()
{
  tmtable[TIMEIO] = (struct timemetric) {0, 0, 0, 0};
  tmtable[TIMEFS] = (struct timemetric) {0, 0, 0, 0};
  tmtable[TIMEMM] = (struct timemetric) {0, 0, 0, 0};

	tptable[THROUGHPUT] = (struct tpmetric) {0, 0, 0, 0, 0};
}

void
metrics_timeadd(uint t, uint64 time)
{
  struct timemetric* tm = &tmtable[t];

  tm->num++;
  tm->total += time;

  if (tm->num == 1 || time < tm->min)
    tm->min = time;
  if (tm->num == 1 || time > tm->max)
    tm->max = time;
}

uint64
metrics_gettimenorm(uint t) {
  struct timemetric* tm = &tmtable[t];
  uint64 norm = (1000 * (tm->num * tm->max - tm->total)) / (tm->num * (tm->max - tm->min));
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

  if (tpmetric->tick_count == 1 || tp < tpmetric->min)
    tpmetric->min = tp;
  if (tpmetric->tick_count == 1 || tp > tpmetric->max)
    tpmetric->max = tp;
}

void
metrics_proc_exited()
{
	++tptable[THROUGHPUT].exited_procs;
}
