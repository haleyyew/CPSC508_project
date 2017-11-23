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

void parity(void* data){
	cprintf("===PARITY===");
	char buf[10];
//	memset(buf, 0, sizeof(buf));

	strncpy(buf, data, 10);
	cprintf("%s", buf);
	cprintf("%s", data);
	cprintf("====== \n");
}
