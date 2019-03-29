#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include "types.h"
#include "defs.h"

int nthr,serverport,commport;
char *rootdir;
pthread_mutex_t mtx2;		//mutex for metadata (number of pages served and bytes served)
struct metadata meta;
struct timespec tv0;
volatile sig_atomic_t flag;


int
main(int argc,char *argv[])
{
	
	
	int status; 
	
	if(argc!=9){
		printf("Wrong arguments1!\n");
		return 0;
	}
	if(strcmp(argv[1],"-p") || strcmp(argv[3],"-c") || strcmp(argv[5],"-t") || strcmp(argv[7],"-d")){
		printf("Wrong arguments2!\n");
		return 0;
	}
		
	serverport=atoi(argv[2]);
	commport=atoi(argv[4]);
	nthr=atoi(argv[6]);
	rootdir=argv[8];
	
	
	pthread_mutex_init(&mtx2,0);
	
	meta.bytes=0;
	meta.pages=0;
	
	clock_gettime(CLOCK_MONOTONIC,&tv0);
	
	
	
	Server((void *)serverport);
	
	
	pthread_mutex_destroy(&mtx2);
	
	return 0;
	
	
}
	
	
