#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include "defs.h"
#include "types.h"
//#define BASEPATH "/tmp/fifo"		//alternative way to store basepath
#define MAXDIR 255
;volatile sig_atomic_t flag2=0;			//global variable for signal handling
extern char **dirs;
extern pid_t *pids;
extern FILE *fp;
extern pthread_mutex_t mtx;
extern pthread_cond_t cond_nonempty;		//cond var for pool
extern pool_t *pool;
extern pthread_t *workers;	

void sigintInterrupt(int signo){
	//printf("Worker with pid %d got signal SIGINT\n",getpid());
	flag2=1;	
}
void sigusrInterrupt(int signo){
	//printf("Worker with pid %d got signal SIGUSR1\n",getpid());
	flag2=2;	
}


int Worker(int l,int bool)		//l=linecount , bool=decide if worker will send to JE buffsize
{
	char *basepath="tmp/fifo",input[1000],logbuff[1000];
	char **buff,**map,**fnames;		//buffer where dirs that worker owns will be stored 
	int i,readfd,writefd,n,counter,maxline,dig,dig2,logfd;
	long msize,wc,buffsize;
	
	memset(logbuff,'\0',1000);
	
	buff=malloc(l*sizeof(char*));
	for(i=0;i<l;i++)
		buff[i]=malloc(MAXDIR*sizeof(char));
	for(i=0;i<l;i++)
		memset(buff[i],'\0',MAXDIR);
	
	

	
	
	
	
	//open pipes and logfile
	readfd=openFifoReadWorker(basepath);		//open pipe and read dir - try to open in while loop cause FIFO maybe hasnt been created yet.
	writefd=openFifoWriteWorker(basepath);
	sprintf(logbuff,"log/Worker_%lu.txt",(long)getpid());
	logfd=open(logbuff,O_WRONLY | O_APPEND);		//open log file
	i=0;
	//Getting dirs from JE and stroing in a 2d array .
	char *temp0=malloc(((l+1)*MAXDIR)*sizeof(char));
	memset(temp0,'\0',(l+1)*MAXDIR);
	if((n=read(readfd,temp0,(l+1)*MAXDIR))<0)
		printf("read error\n");
	kill(getppid(),SIGUSR1);
	char *token=strtok(temp0,"\n");
	i=0;
	while(token!=NULL){
		strcpy(buff[i],token);
		i++;
		token=strtok(NULL,"\n");
		
	}
	free(temp0);
	
	int sumwords=0,sumlines=0;
	//Creating map and insert the words at trie
	map=loadFiles(buff,l,&counter,&msize,&fnames,&wc);
	trieInit();	
	maxline=seperateFiles(map,fnames,counter,msize,&sumlines,&sumwords);	
	
	//Sending at JE how much space to allocate for search queries (buffsize).
	
		dig=noDigits(maxline);
		buffsize=counter*maxline*(512+dig+5)+wc;	//+5 prokiptei apo to protokollo.1 space 2 h 3 newline kai ena nullcharacter
		dig2=noDigits(buffsize);
		char *temp=malloc((dig2+1)*sizeof(char));
		sprintf(temp,"%lu",buffsize);
		temp[dig2]='\0';
		if(bool)
			if(write(writefd,temp,dig2+1)<0)
				perror("write error buffsize");
		free(temp);
	
	char *buffer=malloc(1000*sizeof(char));		//for queries min-max and wc . array of 1000chars is enough for every situation
	memset(buffer,'\0',1000);
	
	navigateWorker(readfd,writefd,counter,buffer,fnames,maxline,buffsize,wc,map,logfd,sumlines,sumwords);
	
	
	
	
	
	dealloc();
	
	//we must free fnames and map
	for(i=0;i<counter;i++){
		free(map[i]);
		free(fnames[i]);
	}
	for(i=0;i<l;i++){
		free(dirs[i]);
		free(buff[i]);
	}
	free(dirs);
	free(buff);
	free(fnames);
	free(map);
	free(buffer);
	//close pipe
	close(readfd);
	close(writefd);
	close(logfd);
	//fclose(fp);
	pthread_mutex_destroy(&mtx);
	pthread_cond_destroy(&cond_nonempty);
	
	struct listnode *p=pool->unique;
	struct listnode *q;
	while(p!=NULL){
		q=p;
		p=p->next;
		free(q);
	}
	
	free(pool);
	free(workers);
	
	
	
	
	exit(0);
}









int noDigits(int i)			//metraei ta psifia enos arithmou
{
	int count=0;
	if(!i)
		return 1;
	while(i>0){
		count++;
		i=i/10;
	}
	return count;
}	








