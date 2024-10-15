#include <kernel/types.h>
#include <kernel/fcntl.h>
#include <user/user.h>

#define NLINES 100
#define NCHAR 100
#define NSWAP 50

#define TMP_FILE "tmp"

void
putc(int fd, char c)
{
  write(fd, &c, 1);  
}

int
main(int argc, char** argv)
{
  char c[255];
  int fd;

  if (argc < 2) strcpy(c, TMP_FILE);
  else strcpy(c, argv[1]);

  if ((fd = open(TMP_FILE, O_RDWR | O_CREATE)) == 0)
    exit(1);

  for (int i = 0; i < NLINES; i++) {
    char buf[NCHAR + 1];

    for (int j = 0; j < NCHAR; j++)
      buf[j] = rng_range(33, 126);
    buf[NCHAR] = '\n';

    write(fd, buf, NCHAR + 1);
  }
  
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

  unlink(TMP_FILE);
  exit(0);
}
