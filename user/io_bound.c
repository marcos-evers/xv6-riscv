#include <kernel/types.h>
#include <kernel/fcntl.h>
#include <user/user.h>

#define NLINES 100
#define NCHAR 100
#define NSWAP 50

int
main(int argc, char** argv)
{
  int fd;

  if ((fd = open("file", O_RDWR | O_CREATE)) == 0)
    exit(1);

  for (int i = 0; i < NLINES; i++) {
    char buf[NCHAR + 1];
    for (int j = 0; j < NCHAR; j++)
      buf[j] = rng_range(33, 126);
    buf[NCHAR] = '\n';
    write(fd, buf, NCHAR + 1);
  }
  
  // TODO: Add swap 50 random lines. May have to implement file seek. I did not found in the source code.

  close(fd);
  exit(0);
}
