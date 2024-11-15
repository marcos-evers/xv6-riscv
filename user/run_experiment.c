#include <kernel/types.h>
#include <kernel/spinlock.h>
#include <kernel/param.h>
#include <kernel/metrics.h>
#include <kernel/riscv.h>
#include <user/user.h>

#define NROUNDS 30
#define NEXPPROC 20
#define CPMS 10000

void
spawn_cpubound(uint ncpu)
{
  int pid;
  char* argv[] = {"cpu_bound", 0};
  for (uint i = 0; i < ncpu; i++) {
    pid = fork();
    if (pid != 0) {
      // inside parent: subscribe child to fairness metric
      msubsproc(pid);
    } else {
      exec(argv[0], argv);
    }
  }
}

void
spawn_iobound(uint nio)
{
  int pid;
  for (uint i = 0; i < nio; i++) {
    pid = fork();
    if (pid != 0) {
      // inside parent: subscribe child to fairness metric
      msubsproc(pid);
    } else {
      char name[] = "tmp_##";

      name[4] = i/10 + '0';
      name[5] = i%10 + '0';

      char* argv[] = {"io_bound", name, 0};

      exec(argv[0], argv);
    }
  }
}

int
main(int argc, char** argv)
{
  uint64 tfs, tmm, fair, et =0;
  for (int i = 1; i <= NROUNDS; i++) {
    uint ncpu = rng_range(3 * NEXPPROC / 10, 7 * NEXPPROC / 10); // X
    uint nio = NEXPPROC - ncpu;                                  // Y

    printf("[ROUND #%d] ", i);
    printf("%d cpu bound process; ", ncpu);
    printf("%d io bound process\n", nio);

    mreset();

    et = uptime();
    spawn_cpubound(ncpu);
    spawn_iobound(nio);
  
    for (uint num = 0; num < NEXPPROC; num++)
      wait(0);
    et = uptime() - et;

    tfs = gettm(TIMEFS);
    tmm = gettm(TIMEMM);
    fair = getfm();

    printf("tfs=%lu ms, tmm=%lu ms, fair=%lu, et=%lu ticks\n", tfs/CPMS, tmm/CPMS, fair, et);
  }
  
  exit(0);
}
