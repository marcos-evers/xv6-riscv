#include "types.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"
#include "metrics.h"

struct timemetric tmtable[] = {
	[TIMEIO] {{ 0 }},
	[TIMEFS] {{ 0 }},
	[TIMEMM] {{ 0 }}
};

struct tpmetric tptable[] = {
	[THROUGHPUT] {{ 0 }}
};

void
metrics_reset(void)
{
	tmtable[TIMEIO] = (struct timemetric){0};
	tmtable[TIMEFS] = (struct timemetric){0};
	tmtable[TIMEMM] = (struct timemetric){0};

	tptable[THROUGHPUT] = (struct tpmetric){0};

  // TODO move this to a metrics_init()
	initlock(&tmtable[TIMEIO].lock, "timeio");
	initlock(&tmtable[TIMEFS].lock, "timefs");
	initlock(&tmtable[TIMEMM].lock, "timemm");
	initlock(&tptable[THROUGHPUT].lock, "throughput");
}

void
metrics_timeadd(uint t, uint64 time)
{
  struct timemetric* tm = &tmtable[t];

	acquire(&tm->lock);

	++tm->num;
	tm->total += time;

	release(&tm->lock);
}

uint64
metrics_gettime(uint t) {
  struct timemetric* tm = &tmtable[t];

	acquire(&tm->lock);
	uint64 avg = 10 * tm->total / tm->num;
	release(&tm->lock);

	return avg;
}

uint64
metrics_gettp(uint t) {
  struct tpmetric* tp = &tptable[t];

	acquire(&tp->lock);
	uint64 avg = 100 * tp->nexited / tp->ntick;
	release(&tp->lock);

	return avg;
}

void
metrics_tick()
{
	struct tpmetric *tpmetric = &tptable[THROUGHPUT];

	acquire(&tpmetric->lock);
	++tpmetric->ntick;
	release(&tpmetric->lock);
}

void
metrics_proc_exited()
{
  // TODO add lock acquire here
	++tptable[THROUGHPUT].nexited;
}
