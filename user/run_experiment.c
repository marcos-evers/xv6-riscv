#include <kernel/types.h>
#include <kernel/spinlock.h>
#include <kernel/metrics.h>
#include <user/user.h>

#define NROUNDS 30
#define NPROC 20

void
spawn_cpubound(uint ncpu)
{
  for (int i = 0; i < ncpu; i++) {
    if (fork() != 0) continue;

    char* argv[] = {"cpu_bound", 0};
    exec(argv[0], argv);
  }
}

void
spawn_iobound(uint nio)
{
  for (int i = 0; i < nio; i++) {
    if (fork() != 0) continue;

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
  for (int i = 1; i <= NROUNDS; i++) {
    uint ncpu = rng_range(3 * NPROC / 10, 7 * NPROC / 10); // X
    uint nio = NPROC - ncpu; // Y

    printf("[ROUND #%d] ", i);
    printf("%d cpu bound process; ", ncpu);
    printf("%d io bound process\n", nio);

    mreset();

    spawn_cpubound(ncpu);
    spawn_iobound(nio);

    for (uint num = 0; num < NPROC; num++)
      wait(0);

    printf("10 * T_lat = %ld\n", mtime(TIMEIO));
    printf("10 * T_mm = %ld\n", mtime(TIMEMM));
    printf("10 * E_fs = %ld\n", mtime(TIMEFS));
    printf("10 * T_put = %ld\n", mtp(THROUGHPUT ));
  }
  exit(0);
}
