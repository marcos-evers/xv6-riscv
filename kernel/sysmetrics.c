#include "types.h"
#include "riscv.h"
#include "defs.h"

uint64
sys_mtime()
{
  int t;
  argint(0, &t);
  uint64 norm = metrics_gettimenorm(t);
  return norm;
}
