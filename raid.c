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

	  printf(1, "--------------------------------------- \n");

	  printf(1, "[main] open() create testfile dev=1 \n");

	  fd = open("testfile", O_CREATE);
	  fd = open("testfile", O_RDWR);

	  printf(1, "[main] before write() testfile buf=%s \n", buf);

	  write(fd, buf, sizeof(buf));

	  error = fstat(fd, stat);

	  itoa(stat->type, type, 10);
	  itoa(stat->dev, dev, 10);
	  itoa(stat->ino, ino, 10);
	  itoa(stat->nlink, nlink, 10);
	  itoa(stat->size, size, 10);
	  if (error < 0){
		  printf(1, "[main] fstat error %d \n", error);

	  }

	  fd = open("testfile", O_RDWR);
	  num = read(fd, buf3, sizeof(buf));

	  printf(1, "[main] after read() num=%d buf3=%s \n", num, buf3);
	  close(fd);

	  printf(1, "----- \n");

	  for (int i = 2; i < 4; i++) {
		  printf(1, "[main] open() create testfile dev=%d \n", i);

		  fd2 = open_backup("testfile", O_CREATE, i);
		  fd2 = open_backup("testfile", O_RDWR, i);

		  printf(1, "[main] before write() testfile dev=%d buf2=%s \n", i, buf2);

		  error = write_backup(fd2, buf2, sizeof(buf2));
		  if (error < 0){
			  printf(1, "[main] write() error %d \n", error);
		  }

		  error = fstat(fd2, stat);

		  itoa(stat->type, type, 10);
		  itoa(stat->dev, dev, 10);
		  itoa(stat->ino, ino, 10);
		  itoa(stat->nlink, nlink, 10);
		  itoa(stat->size, size, 10);
		  if (error < 0){

			  printf(1, "[main] fstat error %d \n", error);
		  }

		  fd2 = open_backup("testfile", O_RDWR, i);
		  num = read(fd2, buf4, sizeof(buf4));

		  printf(1, "[main] after read() dev=%d num=%d buf4=%s \n", i, num, buf4);
		  close(fd2);

		  printf(1, "----- \n");

	  }

	  corrupt_file("testfile", 1);

	  fd = open("testfile", O_RDWR);
	  num = read(fd, buf5, sizeof(buf5));

	  printf(1, "[main] after corrupt_file(): read() dev=1 num=%d buf5=%s \n", num, buf5);
	  close(fd);

	  printf(1, "----- \n");

	  for(int i = 2; i < 4; i++){
		  fd2 = open_backup("testfile", O_RDWR, i);
		  num = read_backup(fd2, buf6, sizeof(buf6));
		  printf(1, "[main] from backup read() dev=%d num=%d buf6=%s \n", i, num, buf6);
	  }


	  fd = open("testfile", O_RDWR);
	  num = write(fd, buf6, sizeof(buf6));
	  printf(1, "[main] from backup write() dev=%d num=%d bytes to dev=1 \n", num);

	  fd = open("testfile", O_RDWR);
	  num = read(fd, buf7, sizeof(buf7));
	  printf(1, "[main] recovered read() dev=1 num=%d buf7=%s \n", num, buf7);
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


	int data_size = 2*BSIZE;
	char *data = malloc(data_size);
	int j;

	for ( j =0; j<data_size; j++){
		data[j] = 0;
	}


	strcpy(&data[0], "BLOCK1");
	strcpy(&data[512], "BLOCK2");


	printf(1, "--------------------------------------- \n");
	printf(1, "[main] data at [0]=%s \n", &data[0]);
	printf(1, "[main] data at [512]=%s \n", &data[512]);



	int half_data_size = BSIZE;
	int block_size = BSIZE;

	char* data_piece1and3 = malloc(half_data_size);
	char* data_piece2and4 = malloc(half_data_size);
	for ( j =0; j<half_data_size; j++){
		data_piece1and3[j] = 0;
	}
	for ( j =0; j<half_data_size; j++){
		data_piece1and3[j] = 0;
	}


	strcpy(&data_piece1and3[0], &data[0]);

	strcpy(&data_piece2and4[0], &data[512]);


	printf(1, "[main] data_piece1 at [0]=%s \n", &data_piece1and3[0]);

	printf(1, "[main] data_piece2 at [0]=%s \n", &data_piece2and4[0]);


	int fd, fd2;
	fd = open("testfile3", O_CREATE);
	fd2 = open_backup("testfile3", O_CREATE, ROOTDEV2);

	fd = open("testfile3", O_RDWR);


	int error;
	error = write(fd, data_piece1and3, half_data_size);
	if (error < 0) {
		printf(1, "[main] write error %d \n", error);
	} else {
		printf(1, "[main] write() %d bytes to testfile3 dev=1 \n", error);
	}

	fd2 = open_backup("testfile3", O_RDWR, ROOTDEV2);
	error = write_backup(fd2, data_piece2and4, half_data_size);
	if (error < 0) {
		printf(1, "[main] write error %d \n", error);
	} else {
		printf(1, "[main] write() %d bytes to testfile3 dev=%d\n", error, ROOTDEV2);
	}

	close(fd);
	close(fd2);

	char* data_read = malloc(data_size);
	for ( j =0; j<data_size; j++){
		data_read[j] = 0;
	}

	fd = open("testfile3", O_RDWR);
	fd2 = open_backup("testfile3", O_RDWR, ROOTDEV2);
	read(fd, &data_read[0], block_size);
	read_backup(fd2, &data_read[512], block_size);


	printf(1, "[main] data_read at [0]=%s [512]=%s \n", &data_read[0], &data_read[512]);


	free(data_piece1and3);
	free(data_piece2and4);
	free(data_read);
	free(data);

	close(fd);
	close(fd2);

	printf(1, "----- \n");

	printf(1, "[main] preparing for parity disk by XOR dev=%d and dev=% \n", ROOTDEV, ROOTDEV2);
	init_block_striping("testfile3", ROOTDEV, ROOTDEV2);


	char *result = malloc(data_size);
	for ( j =0; j<data_size; j++){
		result[j] = 0;
	}

	build_block_striping("testfile3", block_size, result);
		if (error < 0) {
			printf(1, "[main] write error %d \n", error);
		} else {
			printf(1, "[main] write() %d bytes to testfile3 \n", error);
		}

	int i;
	char buf[16];
    for(i=0; i<block_size; ++i){
    	convertBaseVersion(result[i], 2, buf, 8);
    	if (strcmp(buf, "00000000")) {


    	}
    }

    corrupt_file("testfile3", ROOTDEV);

	  fd = open("testfile3", O_RDWR);

	  int num = read(fd, buf, sizeof(buf));
	  printf(1, "[main] after corrupt_file(): read() dev=1 num=%d buf=%s \n", num, buf);
	  close(fd);

    // suppose testfile3 on ROOTDEV is corrupted, now want to restore using the other 2 disks
    error = restore("testfile3", ROOTDEV, sizeof(buf));
	if (error < 0) {
		printf(1, "[main] restore error %d \n", error);
	} else {
		printf(1, "[main] restore() %d bytes to testfile3 on dev=%d \n", error, ROOTDEV);
	}

	for ( j =0; j<16; j++){
		buf[j] = 0;
	}

	fd = open("testfile3", O_RDWR);
	  num = read(fd, buf, sizeof(buf));
	  printf(1, "[main] after restore, read num=%d buf=%s \n", num, buf);
	  close(fd);
}

//http://www.ccodechamp.com/c-program-to-implement-cyclic-redundancy-check-crc/
void test_reed(){				// use circular redundancy check instead, no correction
	char t[32],cs[32];
	char g[]="001110011";		// generator polynomial 1x^2 + 6x^1 + 3x^0
	int a, e;
	int N = strlen(g);
	char t1[32], t2[32];
	printf(1, "--------------------------------------- \n");

	int fd;
	fd = open("testfile6", O_CREATE);
	close(fd);
	fd = open("testfile6", O_RDWR);

	for (int j =0; j<strlen(t); j++){
		t[j] = 0;
	}
	for (int j =0; j<strlen(t1); j++){
		t1[j] = 0;
	}
	for (int j =0; j<strlen(t2); j++){
		t2[j] = 0;
	}

    strcpy(t, "001010011100101");
    printf(1, "[main] Write data : %s \n", t);		// 5x^4 + 2x^3 + 3x^2 + 4x^1 + 5x^0
	a=strlen(t);


	int error;
	error = write(fd, t, a);
	if (error < 0) {
		printf(1, "[main] write error %d \n", error);
	} else {
		printf(1, "[main] write %d bytes to testfile6 \n", error);
	}


	close(fd);
	fd = open("testfile6", O_RDWR);

	error = read_crc(fd, t1, a);
	if (error < 0) {
		printf(1, "[main] read error %d \n", error);
	} else {
		printf(1, "[main] read %d bytes from testfile6: %s \n", error, t1);
	}

	// corrupting data
	strcpy(t1, "00101001111110101101000");			// 1x^4 + 2x^3 + 3x^2 + 7x^1 + 5x^0
	printf(1, "[main] Corrupt data : %s\n",t1);


    crc(cs, t1, g, a, N);
    for(e=0;(e<N-1) && (cs[e]!='1');e++);
	if(e<N-1)
		printf(1, "[main] Error detected %s \n", cs);
	else
		printf(1, "[main] No error detected %s \n", cs);

	close(fd);
	fd = open("testfile6", O_RDWR);

	// just read from disk again
	error = read_crc(fd, t2, a);
	if (error < 0) {
		printf(1, "[main] read error %d \n", error);
	} else {
		printf(1, "[main] read %d bytes from testfile6 \n", error);
	}

	crc(cs, t2, g, a, N);
	for(e=0;(e<N-1) && (cs[e]!='1');e++);
	if(e<N-1)
		printf(1, "[main] Error detected %s \n", cs);
	else
		printf(1, "[main] No error detected %s \n", cs);
}

//raid 1 mirroring
//raid 3 block level striping with parity
//raid 6 block level striping with distributed parity?
//performance test?

int
main(int argc, char *argv[])
{
  int i;

  i = atoi(argv[1]);

  if (i == 1){
	  test_raid1();
  } if (i == 3){
	  test_raid3();
  } if (i == 6){
	  test_reed();
  }


  exit();
}
