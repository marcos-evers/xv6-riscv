#include "types.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "metrics.h"

#define RESET_TIME_METRIC(x)\
  acquire(&x.lock);\
  x.total = x.num = 0;\
  release(&x.lock);


struct timemetric tmtable[] = {
[TIMEFS] {},
[TIMEMM] {}
};

struct fairmetric fairtable[] = {
[FAIRNESS] {}
};

void
metrics_init(void)
{
	initlock(&tmtable[TIMEFS].lock, "timefs");
	initlock(&tmtable[TIMEMM].lock, "timemm");
	initlock(&fairtable[FAIRNESS].lock, "fairness");

  RESET_TIME_METRIC(tmtable[TIMEMM]);
  RESET_TIME_METRIC(tmtable[TIMEFS]);

  for (int i = 0; i < NPROC; i++) {
    fairtable[FAIRNESS].procs[i].pid = 0;
    fairtable[FAIRNESS].procs[i].sched = 0;
  }
  fairtable[FAIRNESS].n_proc = 0;
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
  tm->num++;
	release(&tm->lock);
}

uint64
metrics_gettm(uint t) {
  uint64 tot;
  struct timemetric* tm = &tmtable[t];

	acquire(&tm->lock);
  if (tm->num == 0) tot = 0;
  else tot = tm->total;
	release(&tm->lock);

	return tot;
}

void
metrics_subscribe_proc(int pid)
{
	struct fairmetric *fm = &fairtable[FAIRNESS];
  uint i = fm->n_proc++;

	acquire(&fm->lock);

	if (fm->n_proc < NPROC) {
		fm->procs[i].pid = pid;
		fm->procs[i].sched = 0;
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
    release(&fm->lock);
  } else if (p->sched) {
    panic("fairness metric start");
  } else {
    p->start = r_time();
    p->sched = 1;
    release(&fm->lock);
  }
}

void
metrics_unschedule(int pid)
{
  struct process *p;
	struct fairmetric *fm = &fairtable[FAIRNESS];

	acquire(&fm->lock);

	for (p = fm->procs; p < fm->procs + fm->n_proc && p->pid != pid; p++);

	if (p == fm->procs + fm->n_proc) {
		release(&fm->lock);
	} else if(!p->sched) {
		panic("fairness metric end");
  } else {
    p->time += r_time() - p->start;
    p->start = 0;
    p->sched = 0;
    release(&fm->lock);
  }
}

uint64
metrics_getfm()
{
	uint64 result;
	uint64 sum = 0, sq_sum = 0;

	struct fairmetric *fm = &fairtable[FAIRNESS];

	acquire(&fm->lock);

	uint64 time;
	for (int i = 0; i < fm->n_proc; ++i) {
		time = fm->procs[i].time;
		
		sum += time;
		sq_sum += time * time;
	}

	result = 1000 * sum * sum / (fm->n_proc * sq_sum);

	release(&fm->lock);

	return result;
}

