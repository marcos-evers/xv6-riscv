#include "types.h"
#include "riscv.h"
#include "defs.h"

uint64
sys_mtime(void)
{
  int t;
  argint(0, &t);
  uint64 norm = metrics_gettime(t);
  return norm;
}

uint64
sys_mtp(void)
{
  int t;
  argint(0, &t);
  uint64 norm = metrics_gettp(t);
  return norm;
}

uint64
sys_mreset(void)
{
  metrics_reset();
  return 0;
}
