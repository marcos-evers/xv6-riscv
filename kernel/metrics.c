#include "types.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "metrics.h"

#define RESET_TIME_METRIC(x)\
  acquire(&x.lock);\
  x.total = 0;\
  release(&x.lock);


struct timemetric tmtable[] = {
[TIMEFS] {{ 0 }},
[TIMEMM] {{ 0 }}
};

struct fairmetric fairtable[] = {
[FAIRNESS] {{ 0 }}
};

void
metrics_init(void)
{
	initlock(&tmtable[TIMEFS].lock, "timefs");
	initlock(&tmtable[TIMEMM].lock, "timemm");
	initlock(&fairtable[FAIRNESS].lock, "fairness");
}

void
metrics_reset(void)
{
  RESET_TIME_METRIC(tmtable[TIMEMM]);
  RESET_TIME_METRIC(tmtable[TIMEFS]);

  for (int i = 0; i < NPROC; i++)
    fairtable[FAIRNESS].procs[i].pid = 0;
  fairtable[FAIRNESS].n_proc = 0;
}

void
metrics_timeadd(uint t, uint64 time)
{
  struct timemetric* tm = &tmtable[t];

	acquire(&tm->lock);
	tm->total += time;
	release(&tm->lock);
}

uint64
metrics_gettm(uint t) {
  uint64 tot;
  struct timemetric* tm = &tmtable[t];

	acquire(&tm->lock);
	tot = tm->total;
	release(&tm->lock);

	return tot;
}

void
metrics_subscribe_proc(int pid)
{
	struct fairmetric *fm = &fairtable[FAIRNESS];

	acquire(&fm->lock);

	if (fm->n_proc < NPROC) {
		fm->procs[fm->n_proc++].pid = pid;
	}

	release(&fm->lock);
}

void
metrics_schedule(int pid)
{
  struct process *p;
	struct fairmetric *fm = &fairtable[FAIRNESS];

	acquire(&fm->lock);

  for (p = &fm->procs[0]; p < fm->procs + fm->n_proc && p->pid != pid; p++);

  if (p == fm->procs + fm->n_proc) {
    goto noproc;
  }
  if (p->start != 0) {
    panic("fairness metric start");
  }
  p->start = r_time();

noproc:
	release(&fm->lock);
}

void
metrics_unschedule(int pid)
{
	uint i;
	struct fairmetric *fm = &fairtable[FAIRNESS];

	acquire(&fm->lock);

	for (i = 0; i < fm->n_proc && fm->procs[i].pid != pid; ++i);

	if (i == fm->n_proc) {
		release(&fm->lock);
		return;
	}

	struct process *proc = &fm->procs[i];

	if (proc->start == 0) {
		panic("fairness metric end");
	}

	proc->time += r_time() - proc->start;
	proc->start = 0;

	release(&fm->lock);
}

uint64
metrics_getfm()
{
	uint64 result;
	uint64 sum = 0, sq_sum = 0;

	struct fairmetric *fm = &fairtable[FAIRNESS];

	acquire(&fm->lock);

	uint64 time;
	for (int i = 0; i < fm->n_proc; ++i)
	{
    // TODO review that
		// dividing by 1000. less precision but prevents overflow
		time = fm->procs[i].time / 1000;
		
		sum += time;
		sq_sum += time * time;
	}

	result = 1000 * sum * sum / (fm->n_proc * sq_sum);

	release(&fm->lock);

	return result;
}

