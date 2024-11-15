#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

extern struct proc proc[NPROC];
uint64 seed;

struct {
  uint nrunnable;
  struct proc* proc[NPROC];
  struct spinlock lock;
} procpool;

static uint64
next() {
  uint64 z = (seed += 0x9e3779b97f4a7c15);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
	z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
	return z ^ (z >> 31);
}

static void
procswap(uint i, uint j) {
  if (!holding(&procpool.lock))
    panic("sched -> procswitch -> not holding lock");

  struct proc* p = procpool.proc[i];
  procpool.proc[i] = procpool.proc[j];
  procpool.proc[j] = p;
}

void
schedinit(void)
{
  seed = r_time();
  procpool.nrunnable = 0;
  initlock(&procpool.lock, "procpool");
  for (uint i = 0; i < NPROC; i++)
    procpool.proc[i] = &proc[i];
}

// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run.
//  - swtch to start running that process.
//  - eventually that process transfers control
//    via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  uint64 idx;

  c->proc = 0;
  for(;;){
    // The most recent process to run may have had interrupts
    // turned off; enable them to avoid a deadlock if all
    // processes are waiting.
    intr_on();

    acquire(&procpool.lock);
    if(procpool.nrunnable == 0) {
      // nothing to run; stop running on this core until an interrupt.
      release(&procpool.lock);
      intr_on();
      asm volatile("wfi");
      continue;
    }

    idx = next() % procpool.nrunnable;
    p = procpool.proc[idx];
    acquire(&p->lock);

    procswap(idx, --procpool.nrunnable); 
    p->state = RUNNING;
    release(&procpool.lock);

    c->proc = p;
    metrics_schedule(p->pid);
    swtch(&c->context, &p->context);
    metrics_unschedule(p->pid);
    c->proc = 0;
    release(&p->lock);
  }
}

// Switch to scheduler.  Must hold only p->lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->noff, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  struct proc *p = myproc();

  if(!holding(&p->lock))
    panic("sched p->lock");
  if(mycpu()->noff != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(intr_get())
    panic("sched interruptible");

  intena = mycpu()->intena;
  swtch(&p->context, &mycpu()->context);
  mycpu()->intena = intena;
}

void
updstate(struct proc *p, uint state)
{
  if (!holding(&p->lock)) {
    panic("update proc state");
  }
  if (p->state == state)
    return; /* nothing to do */

  acquire(&procpool.lock);
  uint i;
  for (i = 0; i < NPROC && p != procpool.proc[i]; i++);

  if (p->state == RUNNABLE) procswap(i, --procpool.nrunnable); 
  else if (state == RUNNABLE) procswap(i, procpool.nrunnable++); 
  release(&procpool.lock);
  p->state = state;
}
