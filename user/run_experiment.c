#include <kernel/types.h>
#include <kernel/spinlock.h>
#include <kernel/param.h>
#include <kernel/metrics.h>
#include <user/user.h>

#define NROUNDS 30
#define NEXPPROC 20

void
spawn_cpubound(uint ncpu)
{
  int pid;
  for (int i = 0; i < ncpu; i++) {
    pid = fork();
    if (pid != 0)
    {
      // inside parent: subscribe child to fairness metric
      msubsproc(pid);
      continue;
    }

    char* argv[] = {"cpu_bound", 0};
    exec(argv[0], argv);
  }
}

void
spawn_iobound(uint nio)
{
  int pid;
  for (int i = 0; i < nio; i++) {
    pid = fork();
    if (pid != 0)
    {
      // inside parent: subscribe child to fairness metric
      msubsproc(pid);
      continue;
    }

    char name[] = "tmp_##";
    name[4] = i/10 + '0';
    name[5] = i%10 + '0';

    char* argv[] = {"io_bound", name, 0};
    exec(argv[0], argv);
  }
}

int
main(int argc, char** argv)
{
  // uint64 tp, fn, fs, mm;
  for (int i = 1; i <= NROUNDS; i++) {
    uint ncpu = rng_range(3 * NEXPPROC / 10, 7 * NEXPPROC / 10); // X
    uint nio = NEXPPROC - ncpu;                                  // Y

    printf("[ROUND #%d] ", i);
    printf("%d cpu bound process; ", ncpu);
    printf("%d io bound process\n", nio);

    mreset();

    spawn_cpubound(ncpu);
    spawn_iobound(nio);
  
    for (uint num = 0; num < NEXPPROC; num++)
      wait(0);

    // tp = mtp(THROUGHPUT);
    // fn = mgetfair();
    // fs = mtime(TIMEFS);
    // mm = mtime(TIMEMM);
    //
    // printf("1000 * E_tp = %ld\n", tp);
    // printf("1000 * E_fn = %ld\n", fn);
    // printf("1000 * E_fs = %ld\n", fs);
    // printf("1000 * E_mm = %ld\n", mm);

    // S desconsidera lat
    // printf("1000 * S = %ld\n\n", (tp + fn + fs + mm) / 4);
  }
  
  exit(0);
}
