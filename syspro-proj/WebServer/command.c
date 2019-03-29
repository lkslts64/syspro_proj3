#include <stdio.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include "types.h"
#include "defs.h"

extern pthread_mutex_t mtx2;		//mutex for metadata (number of pages served and bytes served)
extern struct metadata meta;
extern struct timespec tv0;
extern volatile sig_atomic_t flag;

void getCommand(int fd)
{
	struct timespec tv1;
	char buff[1000];
	char *terminate="Server will shutdown\n";
	char *wrong="Wrong command\n";
	char stats[100];
	char shut[100];
	int i;
	
	memset(buff,'\0',1000);
	memset(stats,'\0',100);
	memset(shut,'\0',100);
	if(read(fd,buff,1000)<0)
		perror("read command");
		
	
	for(i=0;i<strlen(buff);i++)
		if(buff[i]<'A' || buff[i]>'z')
			buff[i]='\0';
	sprintf(stats,"STATS\n");			//no need for this
	sprintf(shut,"SHUTDOWN\n");
		
	
	if(!(strcmp(buff,"STATS")) || !(strcmp(buff,stats)) ){
		clock_gettime(CLOCK_MONOTONIC,&tv1);
		int minutes=(tv1.tv_sec-tv0.tv_sec)/60;
		double nano=(double)(tv1.tv_nsec-tv0.tv_nsec)/1000000000.0;
		double secs=(tv1.tv_sec-tv0.tv_sec)-60*minutes;
		if(nano<0){
			nano++;
			secs--;
		}
		secs+=nano;
		
		
		pthread_mutex_lock(&mtx2);
		sprintf(buff,"Server up for %d:%lf, served %d pages, %lu bytes\n",minutes,secs,meta.pages,meta.bytes);
		pthread_mutex_unlock(&mtx2);
		if(write(fd,buff,strlen(buff))<0)
			perror("write");
	}
	else if(!strcmp(buff,"SHUTDOWN") || !strcmp(buff,shut)){
		flag=1;
		if(write(fd,terminate,strlen(terminate))<0)
			perror("write");
	}
	else
		if(write(fd,wrong,strlen(wrong))<0)
			perror("write");
	close(fd);
	
}
