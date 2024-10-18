#include "types.h"
#include "riscv.h"
#include "defs.h"

uint64
sys_mreset(void)
{
  metrics_reset();
  return 0;
}

uint64
sys_mtime(void)
{
  int t;
  argint(0, &t);
  uint64 norm = metrics_gettimenorm(t);
  return norm;
}
