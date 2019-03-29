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
extern int waithr,nthr;


//global vars for jobExecutor.
int *readfd,*writefd,w=5,l,ln;
long maxbuffsize;					//max buffsize for responses
char **buffer,**dirs;					//for responses
pid_t *pids;					//pids of workers
struct pollfd *fds,*fds2;
FILE *fp;
int count=0;


//na epistrefw to string response ( pernwntas to san orisma stin getComand() ).
void getCommand(int fd)
{
	struct timespec tv1;
	char buff[1000],bcopy[1000],* token[13],*saveptr,*delim=" \n";
	char *terminate="Crawler will shutdown\n";
	char *wrong="Wrong command\n",*notdone="crawling hasnt finished yet!\n";
	char stats[100];
	char shut[100];
	int i;
	
	memset(buff,'\0',1000);
	memset(stats,'\0',100);
	memset(shut,'\0',100);
	if(read(fd,buff,1000)<0)
		perror("read command");
		
	
	for(i=0;i<strlen(buff);i++)
		if(buff[i]<' ')
			buff[i]='\0';
	sprintf(stats,"STATS\n");
	sprintf(shut,"SHUTDOWN\n");
		
	
	
	strcpy(bcopy,buff);
	for(i=0;i<13;i++)
		token[i]=NULL;
	i=0;
	token[0]=strtok_r(bcopy,delim,&saveptr);
	while(token[i]!=NULL && i<13){				//vale tis lekseis se ena pinaka
		i++;
		token[i]=strtok_r(NULL,delim,&saveptr);
	}
	
	
	
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
		sprintf(buff,"Crawler up for %d:%lf, downloaded %d pages, %lu bytes\n",minutes,secs,meta.pages,meta.bytes);
		pthread_mutex_unlock(&mtx2);
		if(write(fd,buff,strlen(buff))<0)
			perror("write");
	}
	else if(!strcmp(buff,"SHUTDOWN") || !strcmp(buff,shut)){
		flag=1;
		if(count){
			sendQeury("/exit");
			ReceiveExit();				//send exit to workers so they free the memory alocated.
			free(pids);
			for(i=0;i<ln;i++)
				free(dirs[i]);
			free(dirs);
			//close pipes
			for(i=0;i<w;i++){
				free(buffer[i]);
				close(writefd[i]);
				close(readfd[i]);
			}    
			free(fds2);
			free(fds);
			free(writefd);
			free(readfd);
			free(buffer);
			//fclose(fp);
		}
		if(write(fd,terminate,strlen(terminate))<0)
			perror("write");
	}
	
	else if(!strcmp(token[0],"SEARCH")){
		if(nthr==waithr && count==0){
			jobExecutor();
			count++;					//get in this if statement only once (start workers one time)
		}
		if(nthr==waithr){
			//navigateJE(readfd,writefd,buffer,w,maxbuffsize,pids,l,dirs,buff);
			sendQeury(buff);
			ReceiveSearch(buff);
			for(i=0;i<w;i++)
				if(write(fd,buffer[i],strlen(buffer[i]))<0)
					perror("write");
			//close(fd);
			for(i=0;i<w;i++){
				memset(buffer[i],'\0',maxbuffsize);
			}
		}
		else{
			if(write(fd,notdone,strlen(notdone))<0)
				perror("write");
		}
	}
	else
		if(write(fd,wrong,strlen(wrong))<0)
			perror("write");
			
	close(fd);
	
}
