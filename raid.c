#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"


void test_raid1(){
	  int fd;
	  int fd2;
	  int error;

	  struct stat* stat;
	  stat = malloc(sizeof(struct stat));

	  char type[10];
	  char dev[10];
	  char ino[10];
	  char nlink[10];
	  char size[10];
	  char buf[10];
	  char buf2[10];
	  char buf3[10];
	  char buf4[10];
	  char buf5[10];
	  char buf6[10];
	  char buf7[10];
	  int num;

	  strcpy(buf, "test");
	  strcpy(buf2, "test");

	  printf(1, "[main] create open testfile \n");

	  fd = open("testfile", O_CREATE);
	  fd = open("testfile", O_RDWR);

	  printf(1, "[main] before write testfile buf=%s \n", buf);

	  write(fd, buf, sizeof(buf));

	  error = fstat(fd, stat);

	  itoa(stat->type, type, 10);
	  itoa(stat->dev, dev, 10);
	  itoa(stat->ino, ino, 10);
	  itoa(stat->nlink, nlink, 10);
	  itoa(stat->size, size, 10);
	  if (error == 0){
		  printf(1, "[main] fstat type=%s dev=%s ino=%s nlink=%s size=%s \n", type, dev, ino, nlink, size);
	  }

	  fd = open("testfile", O_RDWR);
	  num = read(fd, buf3, sizeof(buf));

	  printf(1, "[main] after read num=%d buf3=%s \n", num, buf3);
	  close(fd);

	  printf(1, "----- \n");

	  for (int i = 2; i < 4; i++) {
		  printf(1, "[main] create open_backup testfile2 \n");

		  fd2 = open_backup("testfile2", O_CREATE, i);
		  fd2 = open_backup("testfile2", O_RDWR, i);

		  printf(1, "[main] before write_backup testfile2 buf2=%s \n", buf2);

		  error = write_backup(fd2, buf2, sizeof(buf2));
		  if (error < 0){
			  printf(1, "[main] write_backup error %d \n", error);
		  }

		  error = fstat(fd2, stat);

		  itoa(stat->type, type, 10);
		  itoa(stat->dev, dev, 10);
		  itoa(stat->ino, ino, 10);
		  itoa(stat->nlink, nlink, 10);
		  itoa(stat->size, size, 10);
		  if (error == 0){
			  printf(1, "[main] fstat type=%s dev=%s ino=%s nlink=%s size=%s \n", type, dev, ino, nlink, size);
		  } else {
			  printf(1, "[main] fstat error %d \n", error);
		  }

		  fd2 = open_backup("testfile2", O_RDWR, i);
		  num = read(fd2, buf4, sizeof(buf4));

		  printf(1, "[main] after read num=%d buf4=%s \n", num, buf4);
		  close(fd2);

		  printf(1, "----- \n");

	  }

	  corrupt_file("testfile", 1);

	  fd = open("testfile", O_RDWR);
	  num = read(fd, buf5, sizeof(buf5));

	  printf(1, "[main] after corrupt_file read num=%d buf5=%s \n", num, buf5);
	  close(fd);

	  printf(1, "----- \n");

	  for(int i = 2; i < 4; i++){
		  fd2 = open_backup("testfile2", O_RDWR, i);
		  num = read_backup(fd2, buf6, sizeof(buf6));
		  printf(1, "[main] from backup read num=%d buf6=%s \n", num, buf6);
	  }


	  fd = open("testfile", O_RDWR);
	  num = write(fd, buf6, sizeof(buf6));
	  printf(1, "[main] from backup write num=%d bytes to dev 1 \n", num);

	  fd = open("testfile", O_RDWR);
	  num = read(fd, buf7, sizeof(buf7));
	  printf(1, "[main] recovered read num=%d buf7=%s \n", num, buf7);
	  close(fd);
}

//raid 1 mirroring
//raid 3 block level striping with parity
//raid 6 block level striping with distributed parity
//performance test

int
main(int argc, char *argv[])
{
	test_raid1();


  exit();
}
