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
metrics_reset()
{
	tmtable[TIMEIO] = (struct timemetric){0};
	tmtable[TIMEFS] = (struct timemetric){0};
	tmtable[TIMEMM] = (struct timemetric){0};

	tptable[THROUGHPUT] = (struct tpmetric){0};

	fairtable[FAIRNESS] = (struct fairmetric){0};

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

	if (tm->num == 1 || time < tm->min)
		tm->min = time;
	if (tm->num == 1 || time > tm->max)
		tm->max = time;

	release(&tm->lock);
}

uint64
metrics_gettimenorm(uint t) {
  struct timemetric* tm = &tmtable[t];

	acquire(&tm->lock);
	uint64 norm = (1000 * (tm->num * tm->max - tm->total)) / (tm->num * (tm->max - tm->min));
	release(&tm->lock);

	return norm;
}

void
metrics_tick()
{
	struct tpmetric *tpmetric = &tptable[THROUGHPUT];

	acquire(&tpmetric->lock);

  uint tp = tpmetric->exited_procs;

	++tpmetric->tick_count;
  tpmetric->total_exited_procs += tp;
  tpmetric->exited_procs = 0;

  if (tpmetric->tick_count == 1 || tp < tpmetric->min)
    tpmetric->min = tp;
  if (tpmetric->tick_count == 1 || tp > tpmetric->max)
    tpmetric->max = tp;

	release(&tpmetric->lock);
}

void
metrics_proc_exited()
{
	++tptable[THROUGHPUT].exited_procs;
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

uint64
metrics_get_throughput()
{
	struct tpmetric *tp = &tptable[THROUGHPUT];

	acquire(&tp->lock);
	uint64 norm = (1000 * (tp->tick_count * tp->max - tp->total_exited_procs)) / (tp->tick_count * (tp->max - tp->min));
	release(&tp->lock);

	return norm;
}
