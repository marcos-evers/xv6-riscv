#include "types.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "metrics.h"

struct timemetric tmtable[] = {
	[TIMEIO] {{ 0 }},
	[TIMEFS] {{ 0 }},
	[TIMEMM] {{ 0 }}
};

struct tpmetric tptable[] = {
	[THROUGHPUT] {{ 0 }}
};

struct fairmetric fairtable[] = {
		[FAIRNESS]
		{{ 0 } }};

void
metrics_reset(void)
{
	tmtable[TIMEIO] = (struct timemetric){0};
	tmtable[TIMEFS] = (struct timemetric){0};
	tmtable[TIMEMM] = (struct timemetric){0};

	tptable[THROUGHPUT] = (struct tpmetric){0};
	fairtable[FAIRNESS] = (struct fairmetric){0};

  // TODO move this to a metrics_init()
	initlock(&tmtable[TIMEIO].lock, "timeio");
	initlock(&tmtable[TIMEFS].lock, "timefs");
	initlock(&tmtable[TIMEMM].lock, "timemm");

	initlock(&tptable[THROUGHPUT].lock, "throughput");

	initlock(&fairtable[FAIRNESS].lock, "fairness");
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

void
metrics_subscribe_proc(int pid)
{
	struct fairmetric *fm = &fairtable[FAIRNESS];

	acquire(&fm->lock);

	if (fm->n_proc < NPROC)
	{
		fm->procs[fm->n_proc++].pid = pid;
	}

	release(&fm->lock);
}

void
metrics_proc_start_cycle(int pid)
{
	struct fairmetric *fm = &fairtable[FAIRNESS];

	acquire(&fm->lock);

	int i;
	for (i = 0; i < fm->n_proc; ++i)
	{
		if (fm->procs[i].pid == pid)
		{
			if (fm->procs[i].end != 0)
			{
				panic("fairness metric start");
			}

			fm->procs[i].start = r_time();
			break;
		}
	}

	release(&fm->lock);
}

void
metrics_proc_end_cycle(int pid)
{
	struct fairmetric *fm = &fairtable[FAIRNESS];

	acquire(&fm->lock);

	int i;
	for (i = 0; i < fm->n_proc && fm->procs[i].pid != pid; ++i);

	if (i == fm->n_proc)
	{
		release(&fm->lock);
		return;
	}

	struct process *proc = &fm->procs[i];

	if (proc->start == 0)
	{
		panic("fairness metric end");
	}

	proc->end = r_time();

	proc->time += proc->end - proc->start;
	proc->end = proc->start = 0;

	release(&fm->lock);
}

uint64
metrics_get_fairness()
{
	uint64 result;
	uint64 sum = 0, sq_sum = 0;

	struct fairmetric *fm = &fairtable[FAIRNESS];

	acquire(&fm->lock);

	uint64 time;
	for (int i = 0; i < fm->n_proc; ++i)
	{
		// dividing by 1000. less precision but prevents overflow
		time = fm->procs[i].time / 1000;
		
		sum += time;
		sq_sum += time * time;
	}

	result = 1000 * sum * sum / (fm->n_proc * sq_sum);

	release(&fm->lock);

	return result;
}

