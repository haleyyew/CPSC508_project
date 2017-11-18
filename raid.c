#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"


int
main(int argc, char *argv[])
{
  int fd;
  struct stat* stat;
  int error;

  stat = malloc(sizeof(struct stat));
  fd = open_backup("testfile", O_CREATE, 2);
  error = fstat(fd, stat);

  char type[10];
  char dev[10];
  char ino[10];
  char nlink[10];
  char size[10];

  itoa(stat->type, type, 10);
  itoa(stat->dev, dev, 10);
  itoa(stat->ino, ino, 10);
  itoa(stat->nlink, nlink, 10);
  itoa(stat->size, size, 10);

  if (error == 0){
	  printf(1, type); printf(1, "\n"); printf(1, dev); printf(1, "\n"); printf(1, ino); printf(1, "\n"); printf(1, nlink); printf(1, "\n"); printf(1, size); printf(1, "\n");
  }

  fd = open("testfile2", O_CREATE);
  error = fstat(fd, stat);

  itoa(stat->type, type, 10);
  itoa(stat->dev, dev, 10);
  itoa(stat->ino, ino, 10);
  itoa(stat->nlink, nlink, 10);
  itoa(stat->size, size, 10);

  if (error == 0){
	  printf(1, type); printf(1, "\n"); printf(1, dev); printf(1, "\n"); printf(1, ino); printf(1, "\n"); printf(1, nlink); printf(1, "\n"); printf(1, size); printf(1, "\n");
  }

  exit();
}
