//
// File-system system calls.
// Mostly argument checking, since we don't trust
// user code, and calls into file.c and fs.c.
//

#include "types.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "mmu.h"
#include "proc.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"
#include "fcntl.h"
#include "parity.h"
#include "buf.h"

// Fetch the nth word-sized system call argument as a file descriptor
// and return both the descriptor and the corresponding struct file.
static int
argfd(int n, int *pfd, struct file **pf)
{
  int fd;
  struct file *f;

  if(argint(n, &fd) < 0)
    return -1;
  if(fd < 0 || fd >= NOFILE || (f=myproc()->ofile[fd]) == 0)
    return -1;
  if(pfd)
    *pfd = fd;
  if(pf)
    *pf = f;
  return 0;
}

// Allocate a file descriptor for the given file.
// Takes over file reference from caller on success.
static int
fdalloc(struct file *f)
{
  int fd;
  struct proc *curproc = myproc();

  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd] == 0){
      curproc->ofile[fd] = f;
      return fd;
    }
  }
  return -1;
}

int
sys_dup(void)
{
  struct file *f;
  int fd;

  if(argfd(0, 0, &f) < 0)
    return -1;
  if((fd=fdalloc(f)) < 0)
    return -1;
  filedup(f);
  return fd;
}

int
sys_read(void)
{
  struct file *f;
  int n;
  char *p;

  if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
    return -1;
  return fileread(f, p, n);
}

int
sys_read_backup(void)
{
  struct file *f;
  int n;
  char *p;

  if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
    return -1;
  int num = fileread(f, p, n);


  cprintf("sys_read_backup: ");
  cprintf("dev=%d num= %d res=%s", f->ip->dev, num, p);
  cprintf("\n");

  //parity(p);	// check data


  return num;
}

int
sys_write(void)
{
  struct file *f;
  int n;
  char *p;

  if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
    return -1;

  return filewrite(f, p, n);
}

int
sys_write_backup(void)
{
  struct file *f;
  int n;
  char *p;

  if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0){
    return -1;
  }

  cprintf("sys_write_backup: f=%d n=%d p=%s \n", f, n, p);

  return filewrite_backup(f, p, n);
}

int
sys_close(void)
{
  int fd;
  struct file *f;

  if(argfd(0, &fd, &f) < 0)
    return -1;
  myproc()->ofile[fd] = 0;
  fileclose(f);
  return 0;
}

int
sys_fstat(void)
{
  struct file *f;
  struct stat *st;

  if(argfd(0, 0, &f) < 0 || argptr(1, (void*)&st, sizeof(*st)) < 0)
    return -1;
  return filestat(f, st);
}

// Create the path new as a link to the same inode as old.
int
sys_link(void)
{
  char name[DIRSIZ], *new, *old;
  struct inode *dp, *ip;

  if(argstr(0, &old) < 0 || argstr(1, &new) < 0)
    return -1;

  begin_op();
  if((ip = namei(old)) == 0){
    end_op();
    return -1;
  }

  ilock(ip);
  if(ip->type == T_DIR){
    iunlockput(ip);
    end_op();
    return -1;
  }

  ip->nlink++;
  iupdate(ip);
  iunlock(ip);

  if((dp = nameiparent(new, name, ROOTDEV)) == 0)		// root
    goto bad;
  ilock(dp);
  if(dp->dev != ip->dev || dirlink(dp, name, ip->inum) < 0){
    iunlockput(dp);
    goto bad;
  }
  iunlockput(dp);
  iput(ip);

  end_op();

  return 0;

bad:
  ilock(ip);
  ip->nlink--;
  iupdate(ip);
  iunlockput(ip);
  end_op();
  return -1;
}

// Is the directory dp empty except for "." and ".." ?
static int
isdirempty(struct inode *dp)
{
  int off;
  struct dirent de;

  for(off=2*sizeof(de); off<dp->size; off+=sizeof(de)){
    if(readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
      panic("isdirempty: readi");
    if(de.inum != 0)
      return 0;
  }
  return 1;
}

//PAGEBREAK!
int
sys_unlink(void)
{
  struct inode *ip, *dp;
  struct dirent de;
  char name[DIRSIZ], *path;
  uint off;

  if(argstr(0, &path) < 0)
    return -1;

  begin_op();
  if((dp = nameiparent(path, name, ROOTDEV)) == 0){				// root
    end_op();
    return -1;
  }

  ilock(dp);

  // Cannot unlink "." or "..".
  if(namecmp(name, ".") == 0 || namecmp(name, "..") == 0)
    goto bad;

  if((ip = dirlookup(dp, name, &off)) == 0)
    goto bad;
  ilock(ip);

  if(ip->nlink < 1)
    panic("unlink: nlink < 1");
  if(ip->type == T_DIR && !isdirempty(ip)){
    iunlockput(ip);
    goto bad;
  }

  memset(&de, 0, sizeof(de));
  if(writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
    panic("unlink: writei");
  if(ip->type == T_DIR){
    dp->nlink--;
    iupdate(dp);
  }
  iunlockput(dp);

  ip->nlink--;
  iupdate(ip);
  iunlockput(ip);

  end_op();

  return 0;

bad:
  iunlockput(dp);
  end_op();
  return -1;
}

static struct inode*
create(char *path, short type, short major, short minor, uint dev)			// specify dev
{
  uint off;
  struct inode *ip, *dp;
  char name[DIRSIZ];

  if((dp = nameiparent(path, name, dev)) == 0)
    return 0;
  ilock(dp);

  if((ip = dirlookup(dp, name, &off)) != 0){

	//cprintf("create: dirlookup(dp, name, &off)) != 0");

    iunlockput(dp);
    ilock(ip);
    if(type == T_FILE && ip->type == T_FILE)
      return ip;
    iunlockput(ip);
    return 0;
  }

  if((ip = ialloc(dp->dev, type)) == 0)
    panic("create: ialloc");

  ilock(ip);
  ip->major = major;
  ip->minor = minor;
  ip->nlink = 1;
  iupdate(ip);

  if(type == T_DIR){  // Create . and .. entries.
    dp->nlink++;  // for ".."
    iupdate(dp);
    // No ip->nlink++ for ".": avoid cyclic ref count.
    if(dirlink(ip, ".", ip->inum) < 0 || dirlink(ip, "..", dp->inum) < 0)
      panic("create dots");
  }

  if(dirlink(dp, name, ip->inum) < 0)
    panic("create: dirlink");

  iunlockput(dp);

  cprintf("create: path=%s dev=%d \n", path, dev);

  return ip;
}


int
sys_open(void)
{
  char *path;
  int fd, omode;
  struct file *f;
  struct inode *ip;

  if(argstr(0, &path) < 0 || argint(1, &omode) < 0)
    return -1;

  begin_op();

  if(omode & O_CREATE){
    ip = create(path, T_FILE, 0, 0, ROOTDEV);	// root
    cprintf("sys_open: path=%s \n",path);

    if(ip == 0){
      end_op();
      return -1;
    }
  } else {
    if((ip = namei(path)) == 0){
      end_op();
      return -1;
    }
    ilock(ip);
    if(ip->type == T_DIR && omode != O_RDONLY){
      iunlockput(ip);
      end_op();
      return -1;
    }
  }

  if((f = filealloc()) == 0 || (fd = fdalloc(f)) < 0){
    if(f)
      fileclose(f);
    iunlockput(ip);
    end_op();
    return -1;
  }
  iunlock(ip);
  end_op();

  f->type = FD_INODE;
  f->ip = ip;
  f->off = 0;
  f->readable = !(omode & O_WRONLY);
  f->writable = (omode & O_WRONLY) || (omode & O_RDWR);

  cprintf("sys_open fd=%d \n", fd);

  return fd;
}

int
sys_open_backup(void)
{
  char *path;
  int fd, omode;
  struct file *f;
  struct inode *ip;
  int dev;		// specify dev

  if(argstr(0, &path) < 0 || argint(1, &omode) < 0 || argint(2, &dev) < 0)
    return -1;

  cprintf("sys_open_backup: dev=%d pid=%d \n", dev, myproc()->pid);

  begin_op();

  if(omode & O_CREATE){
    ip = create(path, T_FILE, 0, 0, dev);	// specify dev

    cprintf(" sys_open_backup: O_CREATE path= \n", path);

    if(ip == 0){
      end_op();
      return -1;
    }
  } else {
    if((ip = namei_backup(path, dev)) == 0){
      end_op();
      return -1;
    }
    ilock(ip);
    if(ip->type == T_DIR && omode != O_RDONLY){
      iunlockput(ip);
      end_op();
      return -1;
    }
  }

  if((f = filealloc()) == 0 || (fd = fdalloc(f)) < 0){
    if(f)
      fileclose(f);
    iunlockput(ip);
    end_op();
    return -1;
  }
  iunlock(ip);
  end_op();

  f->type = FD_INODE;
  f->ip = ip;
  f->off = 0;
  f->readable = !(omode & O_WRONLY);
  f->writable = (omode & O_WRONLY) || (omode & O_RDWR);


  struct inode *cwd = myproc()->cwd;
  cprintf(" sys_open_backup: cwd->dev=%d name=%s pid=%d \n", cwd->dev, myproc()->name, myproc()->pid);
  //switchuvm(currproc);
  cprintf(" sys_open_backup: fd= \n", fd);

  return fd;
}

int
sys_corrupt_file(void){			// simulate file content corrupt
  char buf[10];
  strncpy(buf, "????", sizeof("????"));

  char *path;
  int fd;
  struct file *f;
  struct inode *ip;
  int dev;

  if(argstr(0, &path) < 0  || argint(1, &dev) < 0)
    return -1;

  begin_op();


	if((ip = namei_backup(path, dev)) == 0){
	  end_op();
	  return -1;
	}
	ilock(ip);


  if((f = filealloc()) == 0 || (fd = fdalloc(f)) < 0){
    if(f)
      fileclose(f);
    iunlockput(ip);
    end_op();
    return -1;
  }
  iunlock(ip);
  end_op();

  f->type = FD_INODE;
  f->ip = ip;
  f->off = 0;
  f->readable = 1;
  f->writable = 1;

  cprintf("sys_corrupt_file: buf=%s \n", buf);

  filewrite(f, buf, sizeof(buf));

  return 0;
}

int
sys_mkdir(void)
{
  char *path;
  struct inode *ip;

  begin_op();
  if(argstr(0, &path) < 0 || (ip = create(path, T_DIR, 0, 0, ROOTDEV)) == 0){	// root
    end_op();
    return -1;
  }
  iunlockput(ip);
  end_op();
  return 0;
}

int
sys_mknod(void)
{
  struct inode *ip;
  char *path;
  int major, minor;

  begin_op();
  if((argstr(0, &path)) < 0 ||
     argint(1, &major) < 0 ||
     argint(2, &minor) < 0 ||
     (ip = create(path, T_DEV, major, minor, ROOTDEV)) == 0){	// root
    end_op();
    return -1;
  }
  iunlockput(ip);
  end_op();
  return 0;
}

int
sys_chdir(void)
{
  char *path;
  struct inode *ip;
  struct proc *curproc = myproc();
  
  begin_op();
  if(argstr(0, &path) < 0 || (ip = namei(path)) == 0){
    end_op();
    return -1;
  }
  ilock(ip);
  if(ip->type != T_DIR){
    iunlockput(ip);
    end_op();
    return -1;
  }
  iunlock(ip);
  iput(curproc->cwd);
  end_op();
  curproc->cwd = ip;
  return 0;
}

int
sys_exec(void)
{
  char *path, *argv[MAXARG];
  int i;
  uint uargv, uarg;

  if(argstr(0, &path) < 0 || argint(1, (int*)&uargv) < 0){
    return -1;
  }
  memset(argv, 0, sizeof(argv));
  for(i=0;; i++){
    if(i >= NELEM(argv))
      return -1;
    if(fetchint(uargv+4*i, (int*)&uarg) < 0)
      return -1;
    if(uarg == 0){
      argv[i] = 0;
      break;
    }
    if(fetchstr(uarg, &argv[i]) < 0)
      return -1;
  }
  return exec(path, argv);
}

int
sys_pipe(void)
{
  int *fd;
  struct file *rf, *wf;
  int fd0, fd1;

  if(argptr(0, (void*)&fd, 2*sizeof(fd[0])) < 0)
    return -1;
  if(pipealloc(&rf, &wf) < 0)
    return -1;
  fd0 = -1;
  if((fd0 = fdalloc(rf)) < 0 || (fd1 = fdalloc(wf)) < 0){
    if(fd0 >= 0)
      myproc()->ofile[fd0] = 0;
    fileclose(rf);
    fileclose(wf);
    return -1;
  }
  fd[0] = fd0;
  fd[1] = fd1;
  return 0;
}

#define BLOCK_STRIPING_START_ADDR 24

int sys_init_block_striping(void){
  char *path;
  int dev1;
  int dev2;

  struct inode *ip;
  struct file *f;

  if(argstr(0, &path) < 0 || argint(1, &dev1) < 0 || argint(2, &dev2) < 0)
    return -1;

  cprintf("sys_init_block_striping: path=%s dev1=%d dev2=%d \n", path, dev1, dev2);

  begin_op();

  ip = create(path, T_FILE, 0, 0, ROOTDEVBKUP);

	if(ip == 0){
	  end_op();
	  return -1;
	}

  if((f = filealloc()) == 0){
    iunlockput(ip);
    end_op();
    return -1;
  }
  iunlock(ip);
  end_op();

  f->type = FD_INODE;
  f->ip = ip;
  f->off = 0;
  f->readable = 1;
  f->writable = 1;

  cprintf("sys_init_block_striping: ip->inum=%d f->ip->inum=%d \n", ip->inum, f->ip->inum);

  char buf[16];
  char header[24];
  int j;
	for ( j =0; j<16; j++){
		buf[j] = 0;
	}
	for ( j =0; j<24; j++){
		header[j] = 0;
	}

  itoa(dev1, buf, 10);
  strncpy(header, buf, 8);
  itoa(dev2, buf, 10);
  strncpy(&header[8], buf, 8);

  cprintf("sys_init_block_striping: header 0=%s 8=%s \n", header, &header[8]);

  int error;
  begin_op();
  ilock(f->ip);
  error = writei(f->ip, header, f->off, BLOCK_STRIPING_START_ADDR);
  iunlock(f->ip);
  end_op();

  if (error < 0)
	  return error;

  char head_read[24];
	for ( j =0; j<24; j++){
		head_read[j] = 0;
	}
  ilock(f->ip);
  error = readi(ip, head_read, 0, BLOCK_STRIPING_START_ADDR);
  iunlock(f->ip);

  if (error < 0)
	  return error;

  cprintf("sys_init_block_striping: head_read 0=%s 8=%s \n", head_read, &head_read[8]);

  return error;
}

int sys_build_block_striping (void){
  char *path;
  int dev1size;

  char *result;

  struct inode *ip;
  struct file *f;
  struct file *f1;
  struct file *f2;

  if(argstr(0, &path) < 0 || argint(1, &dev1size) < 0  || argptr(2, &result, dev1size) < 0)
	return -1;

  int dev1;
  int dev2;

  cprintf("sys_build_block_striping path=%s \n", path);

  // open init block_striping from backup
  begin_op();
  if((ip = namei_backup(path, 2)) == 0){
    end_op();
    return -1;
  }
  end_op();

  begin_op();
  ilock(ip);
	if((f = filealloc()) == 0){
	  if(f)
		fileclose(f);
	  iunlockput(ip);
	  end_op();
	  return -1;
	}
	iunlock(ip);
	end_op();

	f->type = FD_INODE;
	f->ip = ip;
	f->off = 0;
	f->readable = 1;
	f->writable = 1;

  cprintf("sys_build_block_striping ip->inum %d \n", ip->inum);

  // read the header
//  uint off= 0;
//  uint tot= 0;
//  uint m = 0;
//  struct buf *bp;
//  int n = BLOCK_STRIPING_START_ADDR;
  char dst [32];		// 4 KB
  int j;
	for ( j =0; j<32; j++){
		dst[j] = 0;
	}

  ilock(f->ip);
//  for(tot=0; tot<n; tot+=m, off+=m, dst+=m){
//      bp = bread(ip->dev, bmap_backup(ip, off/BSIZE));
//      m = min_backup(n - tot, BSIZE - off%BSIZE);
//
//      cprintf("sys_build_block_striping m=%d \n", m);
//
//      memmove(dst, bp->data + off%BSIZE, m);
//      brelse(bp);
//  }

  readi(ip, dst, 0, BLOCK_STRIPING_START_ADDR);
  iunlock(f->ip);

  char header[24];
	for ( j =0; j<24; j++){
		header[j] = 0;
	}
  strncpy(header, &dst[0], 8);
  strncpy(&header[8], &dst[8], 8);

  //kfree(dst);

  dev1 = atoi(header);
  dev2 = atoi(&header[8]);
  cprintf("sys_build_block_striping: dst 0=%d 8=%d \n", dev1, dev2);

  // read from odd-numbered block pieces from dev 1
  struct inode *ip1;
  begin_op();
  if((ip1 = namei_backup(path, ROOTDEV)) == 0){
    end_op();
    return -1;
  }
  f1 = filealloc();
  end_op();
  f1->type = FD_INODE;
  f1->ip = ip1;
  f1->off = 0;
  f1->readable = 1;
  f1->writable = 1;

  cprintf("sys_build_block_striping ip1->inum %d \n", ip1->inum);

  // read from odd-numbered block pieces from dev 2
  struct inode *ip2;
  begin_op();
  if((ip2 = namei_backup(path, ROOTDEV2)) == 0){
    end_op();
    return -1;
  }
  f2 = filealloc();
  end_op();
  f2->type = FD_INODE;
  f2->ip = ip2;
  f2->off = 0;
  f2->readable = 1;
  f2->writable = 1;

  cprintf("sys_build_block_striping ip2->inum %d \n", ip2->inum);

//  char dst1[dev1size];
//  ilock(ip1);
//  readi(ip1, dst1, 0, dev1size);
//  iunlock(ip1);
//
//  cprintf("sys_build_block_striping 0=%s 512=%s \n", &dst1[0], &dst1[BSIZE]);

//  char dst2[dev2size];		// due to limitations of num of blocks per process, cannot use dst2
//  cprintf("sys_build_block_striping 0=%s 512=%s \n", &dst1[0], &dst1[BSIZE]);

  int tot;
  char dst1[BSIZE];
  char dst2[BSIZE];
	for ( j =0; j<BSIZE; j++){
		dst1[j] = 0;
	}
	for ( j =0; j<BSIZE; j++){
		dst2[j] = 0;
	}
  int error = -1;
  //char* result_tmp = kalloc();

  for(tot=0; tot<dev1size; tot+=BSIZE){
	  ilock(ip1);
	  readi(ip1, dst1, tot, BSIZE);
	  iunlock(ip1);

	  ilock(ip2);
	  readi(ip2, dst2, tot, BSIZE);
	  iunlock(ip2);

	  parity(dst1, dst2, result+tot);
  }

  fileclose(f1);
  fileclose(f2);

  begin_op();
  ilock(ip);
  error = writei(ip, result, BLOCK_STRIPING_START_ADDR, dev1size);
  iunlock(ip);
  end_op();

  return error;
}

int
sys_restore(){
	  char *path;
	  int dev;
	  int size;

	  struct inode *ip;
	  struct file *f1;
	  struct file *f2;

	  if(argstr(0, &path) < 0 || argint(1, &dev) < 0 || argint(2, &size) < 0)
		return -1;

	  int dev1;
	  int dev2;
	  int off;

	  if (dev == ROOTDEV){
		  dev1 = ROOTDEVBKUP;
		  off = BLOCK_STRIPING_START_ADDR;
		  dev2 = ROOTDEV2;
	  }

	  struct inode *ip1;
	  begin_op();
	  if((ip1 = namei_backup(path, dev1)) == 0){
	    end_op();
	    return -1;
	  }
	  f1 = filealloc();
	  end_op();
	  f1->type = FD_INODE;
	  f1->ip = ip1;
	  f1->off = 0;
	  f1->readable = 1;
	  f1->writable = 1;

	  cprintf("sys_restore ip1->inum %d \n", ip1->inum);

	  // read from odd-numbered block pieces from dev 2
	  struct inode *ip2;
	  begin_op();
	  if((ip2 = namei_backup(path, dev2)) == 0){
	    end_op();
	    return -1;
	  }
	  f2 = filealloc();
	  end_op();
	  f2->type = FD_INODE;
	  f2->ip = ip2;
	  f2->off = 0;
	  f2->readable = 1;
	  f2->writable = 1;

	  cprintf("sys_restore ip2->inum %d \n", ip2->inum);

	  int j;
	  char dst1[BSIZE];
	  char dst2[BSIZE];
	  char result[BSIZE];
		for ( j =0; j<BSIZE; j++){
			dst1[j] = 0;
		}
		for ( j =0; j<BSIZE; j++){
			dst2[j] = 0;
		}
		for ( j =0; j<BSIZE; j++){
			result[j] = 0;
		}

	  int tot;
	  for(tot=0; tot<size; tot+=BSIZE){
		  ilock(ip1);
		  readi(ip1, dst1, tot+off, BSIZE);
		  iunlock(ip1);

		  ilock(ip2);
		  readi(ip2, dst2, tot, BSIZE);
		  iunlock(ip2);

		  parity(dst1, dst2, result+tot);
	  }
	  cprintf("sys_restore dst1 0=%s 16=%s  \n", dst1, dst1[24]);


	  fileclose(f1);
	  fileclose(f2);

	  begin_op();
	  if((ip = namei_backup(path, dev)) == 0){
	    end_op();
	    return -1;
	  }
	  f1 = filealloc();
	  end_op();
	  f1->type = FD_INODE;
	  f1->ip = ip;
	  f1->off = 0;
	  f1->readable = 1;
	  f1->writable = 1;

	  int error;
	  begin_op();
	  ilock(ip);
	  error = writei(ip, result, 0, sizeof(result));
	  iunlock(ip);
	  end_op();

	  return error;

}
