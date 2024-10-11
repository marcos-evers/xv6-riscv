#include <kernel/types.h>
#include <kernel/fcntl.h>
#include <user/user.h>

#define NLINES 100
#define NCHAR 100
#define NSWAP 50

void
putc(int fd, char c)
{
  write(fd, &c, 1);  
}

int
main(int argc, char** argv)
{
  int fd;

  if ((fd = open("file", O_RDWR | O_CREATE)) == 0)
    exit(1);

  for (int i = 0; i < NLINES; i++) {
    putc(fd, '0' + i);
    for (int j = 1; j < NCHAR; j++)
      putc(fd, rng_range(33, 126));
    putc(fd, '\n');
  }
  
  // TODO: Add swap 50 random lines.
  for (int it = 0; it < NSWAP; it++) {
    int i = rng_range(0, NLINES - 1); 
    int j = rng_range(0, NLINES - 2);

    if (j >= i) j++;

    char buf_i[NCHAR], buf_j[NCHAR];

    lseek(fd, i * (NCHAR + 1), SEEK_SET);
    read(fd, buf_i, NCHAR);

    lseek(fd, j * (NCHAR + 1), SEEK_SET);
    read(fd, buf_j, NCHAR);

    lseek(fd, i * (NCHAR + 1), SEEK_SET);
    write(fd, buf_j, NCHAR);

    lseek(fd, j * (NCHAR + 1), SEEK_SET);
    write(fd, buf_i, NCHAR);
  }

  close(fd);
  exit(0);
}
