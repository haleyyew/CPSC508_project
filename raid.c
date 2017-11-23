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

  char type[10];
  char dev[10];
  char ino[10];
  char nlink[10];
  char size[10];

  char buf[10];
  //memset(buf, 0, sizeof(buf));

  strcpy(buf, "tes");

  printf(1, "----- \n");

  error = fstat(fd, stat);
  itoa(stat->type, type, 10);
  itoa(stat->dev, dev, 10);
  itoa(stat->ino, ino, 10);
  itoa(stat->nlink, nlink, 10);
  itoa(stat->size, size, 10);
  if (error == 0){
	  //printf(1, type); printf(1, "\n"); printf(1, dev); printf(1, "\n"); printf(1, ino); printf(1, "\n"); printf(1, nlink); printf(1, "\n"); printf(1, size); printf(1, "\n");
  }
  //memset(type, 0, sizeof(type));

  printf(1, "----- \n");

  fd = open_backup("testfile", O_RDWR, 2);

  printf(1, "SIZEOF test %d \n", sizeof("tes"));
  printf(1, "BEFORE WRITE 	%s \n", buf);
  printf(1, "BEFORE WRITE 	%x \n", buf);
  write(fd, buf, sizeof(buf));

  error = fstat(fd, stat);
  itoa(stat->type, type, 10);
  itoa(stat->dev, dev, 10);
  itoa(stat->ino, ino, 10);
  itoa(stat->nlink, nlink, 10);
  itoa(stat->size, size, 10);
  if (error == 0){
	  printf(1, type); printf(1, "\n"); printf(1, dev); printf(1, "\n"); printf(1, ino); printf(1, "\n"); printf(1, nlink); printf(1, "\n"); printf(1, size); printf(1, "\n");
  }

  fd = open_backup("testfile", O_RDWR, 2);
  int num = read_backup(fd, type, sizeof(buf));
  printf(1, "read_backup %d \n", num);
  printf(1, "read_backup %s \n", type);


  printf(1, "----- \n");

  fd = open("testfile2", O_CREATE);

  fd = open("testfile2", O_RDWR);

  //memset(type, 0, sizeof(type));
  //memset(buf, 0, sizeof(buf));
  strcpy(buf, "aha");

  printf(1, "BEFORE WRITE 	%s \n", buf);
  write(fd, buf, sizeof(buf));

  error = fstat(fd, stat);
  itoa(stat->type, type, 10);
  itoa(stat->dev, dev, 10);
  itoa(stat->ino, ino, 10);
  itoa(stat->nlink, nlink, 10);
  itoa(stat->size, size, 10);
  if (error == 0){
	  printf(1, type); printf(1, "\n"); printf(1, dev); printf(1, "\n"); printf(1, ino); printf(1, "\n"); printf(1, nlink); printf(1, "\n"); printf(1, size); printf(1, "\n");
  }

  fd = open("testfile2", O_RDWR);
  num = read_backup(fd, type, sizeof(buf));
  printf(1, "read_backup %d \n", num);
  printf(1, "read_backup %s \n", type);

  exit();
}
