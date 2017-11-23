#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"


//raid 1 mirroring
//raid 3 block level striping with parity
//raid 6 block level striping with distributed parity
//performance test

int
main(int argc, char *argv[])
{
  int fd;
  int fd2;
  struct stat* stat;
  int error;

  stat = malloc(sizeof(struct stat));

  char type[10];
  char dev[10];
  char ino[10];
  char nlink[10];
  char size[10];

  char buf[10];
  char buf2[10];
  int num;
  //memset(buf, 0, sizeof(buf));

  printf(1, "----- \n");

  fd = open("testfile", O_CREATE);

  fd = open("testfile", O_RDWR);

  //memset(type, 0, sizeof(type));
  //memset(buf, 0, sizeof(buf));
  strcpy(buf, "test");

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

  fd = open("testfile", O_RDWR);
  num = read(fd, type, sizeof(buf));
  printf(1, "read %d \n", num);
  printf(1, "read %s \n", type);
  close(fd);

  printf(1, "----- \n");

  strcpy(buf2, "test");

  fd2 = open_backup("testfile", O_CREATE, 2);


  fd2 = open_backup("testfile", O_RDWR, 2);

//  printf(1, "SIZEOF test %d \n", sizeof("test"));
  printf(1, "BEFORE WRITE 	%s \n", buf2);

  write_backup(fd2, buf2, sizeof(buf2));

  error = fstat(fd2, stat);
  itoa(stat->type, type, 10);
  itoa(stat->dev, dev, 10);
  itoa(stat->ino, ino, 10);
  itoa(stat->nlink, nlink, 10);
  itoa(stat->size, size, 10);
  if (error == 0){
	  printf(1, type); printf(1, "\n"); printf(1, dev); printf(1, "\n"); printf(1, ino); printf(1, "\n"); printf(1, nlink); printf(1, "\n"); printf(1, size); printf(1, "\n");
  } else {
	  printf(1, "error %d \n", error);
  }


  fd2 = open_backup("testfile", O_RDWR, 2);

  num = read(fd2, type, sizeof(type));

  printf(1, "read_backup %d \n", num);
  printf(1, "read_backup %s \n", type);

  close(fd2);


  printf(1, "----- \n");
  printf(1, "----- \n");

  corrupt_file("testfile", 1);
  fd = open("testfile", O_RDWR);
  num = read(fd, buf2, sizeof(buf2));
  printf(1, "read %d \n", num);
  printf(1, "read %s \n", buf2);
  close(fd);

  printf(1, "----- \n");

  fd2 = open_backup("testfile", O_RDWR, 2);
  num = read_backup(fd2, type, sizeof(type));
  printf(1, "read_backup %d \n", num);
  printf(1, "read_backup %s \n", type);

  fd = open("testfile", O_RDWR);
  write(fd, type, sizeof(type));
  printf(1, "testfile %d \n", num);
  fd = open("testfile", O_RDWR);
  num = read(fd, type, sizeof(type));
  printf(1, "read %d \n", num);
  printf(1, "read %s \n", type);
  close(fd);

  exit();
}
