#include <kernel/types.h>
#include <kernel/metrics.h>
#include <user/user.h>

int main(int argc, char** argv) {
  if (fork() == 0) {
    char* argv[] = {"io_bound", "tmp", 0};
    exec("io_bound", argv);
  } else {
    wait(0);
    printf("%ld\n", mtime(TIMEIO));
  }
  exit(0);
}
