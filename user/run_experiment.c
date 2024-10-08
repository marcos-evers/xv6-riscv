#include <kernel/types.h>
#include <user/user.h>

int rng_state = 1729;

int main(int argc, char** argv) {
  if(argc != 2)
    exit(1);
  int n = atoi(argv[1]);
  printf("%d\n", n);
  for (uint i = 0; i < n; i++) {
    printf("%u\n", rng());
  }
  exit(0);
}
