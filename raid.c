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
		  printf(1, "[main] create open_backup testfile \n");

		  fd2 = open_backup("testfile", O_CREATE, i);
		  fd2 = open_backup("testfile", O_RDWR, i);

		  printf(1, "[main] before write_backup testfile buf2=%s \n", buf2);

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

		  fd2 = open_backup("testfile", O_RDWR, i);
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
		  fd2 = open_backup("testfile", O_RDWR, i);
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

#define BSIZE 512;  // block size
//#define NDIRECT 12	// num of direct blocks
//#define NINDIRECT (BSIZE / sizeof(uint))	// num of indirect blocks
//#define NINODE 50;	// maximum number of active i−nodes
#define ROOTDEV       1  // device number of file system root disk
#define ROOTDEVBKUP   2  // device number of back up disk
#define ROOTDEV2	  3  // device number of file system root disk 2



void test_raid3(){

	int data_size = 4*BSIZE;
	char data[data_size];

	strcpy(&data[0], "BLOCK1");
	strcpy(&data[512], "BLOCK2");
	strcpy(&data[1024], "BLOCK3");
	strcpy(&data[1536], "BLOCK4");

	printf(1, "[main] data at 0=%s \n", &data[0]);
	printf(1, "[main] data at 512=%s \n", &data[512]);
	printf(1, "[main] data at 1024=%s \n", &data[1024]);
	printf(1, "[main] data at 1536=%s \n", &data[1536]);

	int half_data_size = 2*BSIZE;
	int block_size = BSIZE;

	char* data_piece1and3 = malloc(half_data_size);
	char* data_piece2and4 = malloc(half_data_size);

	strcpy(&data_piece1and3[0], &data[0]);
	strcpy(&data_piece1and3[512], &data[1024]);
	strcpy(&data_piece2and4[0], &data[512]);
	strcpy(&data_piece2and4[512], &data[1536]);

	printf(1, "[main] data_piece1and3 at 0=%s \n", &data_piece1and3[0]);
	printf(1, "[main] data_piece1and3 at 512=%s \n", &data_piece1and3[512]);
	printf(1, "[main] data_piece2and4 at 0=%s \n", &data_piece2and4[0]);
	printf(1, "[main] data_piece2and4 at 512=%s \n", &data_piece2and4[512]);

	int fd, fd2;
	fd = open("testfile", O_CREATE);
	fd2 = open_backup("testfile", O_CREATE, ROOTDEV2);
	fd = open("testfile", O_RDWR);
	fd2 = open_backup("testfile", O_RDWR, ROOTDEV2);

	int error;
	error = write(fd, data_piece1and3, half_data_size);
	if (error < 0) {
		printf(1, "[main] write error %d \n", error);
	} else {
		printf(1, "[main] write %d bytes to testfile \n", error);
	}

	error = write_backup(fd2, data_piece2and4, half_data_size);
	if (error < 0) {
		printf(1, "[main] write error %d \n", error);
	} else {
		printf(1, "[main] write %d bytes to testfile \n", error);
	}

	close(fd);
	close(fd2);

	char* data_read = malloc(data_size);
	fd = open("testfile", O_RDWR);
	fd2 = open_backup("testfile", O_RDWR, ROOTDEV2);
	read(fd, &data_read[0], block_size);
	read_backup(fd2, &data_read[512], block_size);
	read(fd, &data_read[1024], block_size);
	read_backup(fd2, &data_read[1536], block_size);
	printf(1, "[main] data_read at 0=%s 512=%s 1024=%s 1536=%s \n", &data_read[0], &data_read[512], &data_read[1024], &data_read[1536]);

	free(data_piece1and3);
	free(data_piece2and4);
	free(data_read);

	close(fd);
	close(fd2);

	init_block_striping("testfile", ROOTDEV, ROOTDEV2);

	//printf(1, "[main] sizeof data pieces=%d %d \n", sizeof(data_piece1and3), sizeof(data_piece2and4));
	//char* result = malloc(half_data_size);
	//build_block_striping("testfile", half_data_size, half_data_size);
	build_block_striping("testfile", block_size, block_size);

}


//raid 1 mirroring
//raid 3 block level striping with parity
//raid 6 block level striping with distributed parity
//performance test

int
main(int argc, char *argv[])
{
	//test_raid1();
	test_raid3();

  exit();
}
