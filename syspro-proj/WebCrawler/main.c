#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include "types.h"
#include "defs.h"

char *savedir,*hostip,*startURL;
int commport,port,nthr;
pthread_mutex_t mtx2;		//mutex for metadata (number of pages served and bytes served)
struct metadata meta;
struct timespec tv0;
volatile sig_atomic_t flag=0;


int
main(int argc,char *argv[])
{
	if(argc!=12){
		printf("Wrong arguments...\n");
		return 0;
	}
	if(strcmp(argv[1],"-h") || strcmp(argv[3],"-p") || strcmp(argv[5],"-c") || strcmp(argv[7],"-t") || strcmp(argv[9],"-d")){
		printf("Wrong arguments...\n");
		return 0;
	}
	port=atoi(argv[4]);
	commport=atoi(argv[6]);
	nthr=atoi(argv[8]);
	savedir=argv[10];
	startURL=argv[11];
	hostip=argv[2];
	
	pthread_mutex_init(&mtx2,0);
	
	meta.bytes=0;
	meta.pages=0;
	
	
	clock_gettime(CLOCK_MONOTONIC,&tv0);
	
	Crawler();
	
	pthread_mutex_destroy(&mtx2);
	
	return 0;
	
}
	
