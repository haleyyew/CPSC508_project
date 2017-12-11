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

//https://www.techworld.com.au/article/527677/how_convert_an_ascii_char_binary_string_representation_c/
void convertBaseVersion(char input, int base, char *output, int digits)
{
	int i, remainder;
	char digitsArray[17] = "0123456789ABCDEF";

	for (i = digits; i > 0; i--)
	{
		remainder = input % base;
		input = input / base;
		output[i - 1] = digitsArray[remainder];
	}
	output[digits] = '\0';
}

//#define LENGTH 16

void parity(char* data1, char* data2, char* xor_result){
	cprintf("===PARITY=== \n");
	char buf[10];
	//strncpy(buf, data1, 10);
	//cprintf("%s", buf);
//	cprintf("%s \n", data1);
//	cprintf("%s \n", data2);

	//https://stackoverflow.com/questions/27914271/how-can-i-bitwise-xor-two-c-char-arrays
//    char const plainone[LENGTH] = "PO";
//    char const plaintwo[LENGTH] = "PT";
//    char xor[LENGTH];
//    int i;
//
//    for(i=0; i<LENGTH; ++i)
//        xor[i] = (char)(plainone[i] ^ plaintwo[i]);
//    //cprintf("PlainText One: %s\nPlainText Two: %s\n\none^two: ", plainone, plaintwo);
//    for(i=0; i<LENGTH; ++i){
//    	convertBaseVersion(xor[i], 2, xor_result, 8);
//        cprintf("%s ", xor_result);
//    }
//    cprintf("\n");

	//sizeof(data)/BSIZE

    int i;

    for(i=0; i<BSIZE; ++i)
    	xor_result[i] = (char)(data1[i] ^ data2[i]); 	// XOR block 1 and block 2
    for(i=0; i<BSIZE; ++i){
    	convertBaseVersion(xor_result[i], 2, buf, 8);
    	if (strncmp(buf, "00000000", sizeof("00000000"))) {
            cprintf("i=%d binary=%s data1=%s data2=%s result=%s \n", i, buf, &data1[i], &data2[i], xor_result[i]);
    	}
    }
    cprintf("\n");
}

void xor(char* cs, char* g, int N){
	int c;
    for(c = 1;c < N; c++)
    cs[c] = (( cs[c] == g[c])?'0':'1');
}

void crc(char* cs, char* t, char* g, int a, int N){
	int e, c;
    for(e=0;e<N;e++)
        cs[e]=t[e];
    do{
        if(cs[0]=='1')
            xor(cs, g, N);
        for(c=0;c<N-1;c++)
            cs[c]=cs[c+1];
        cs[c]=t[e++];
    }while(e<=a+N-1);
}

int circular_redundancy_check_encode(char* t){
	char g[]="001110011";		// generator polynomial 1x^2 + 6x^1 + 3x^0
	char cs[32];
	int a, e;
	int N = strlen(g);

    cprintf( "\nGenerator polynomial : %s",g);
    a=strlen(t);
    for(e=a;e<a+N-1;e++)
        t[e]='0';

    cprintf( "\nModified data is : %s",t);

    crc(cs, t, g, a, N);
    cprintf( "\nChecksum is : %s",cs);
    for(e=a;e<a+N-1;e++)
        t[e]=cs[e-a];

    cprintf( "\nFinal codeword is : %s\n",t);

    int n = strlen(t);

    return n;
}
