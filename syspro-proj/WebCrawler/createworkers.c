#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "defs.h"
#define MAXDIR 255
#define PERMS 0666
extern volatile sig_atomic_t flag1;
//jobExecutor calls this
pid_t * createWorkers(int w,int l)		//l=lines 
{
	int i,pid;
	pid_t *pids;
	
	pids=malloc(w*sizeof(pid_t));				
	for(i=0;i<w;i++)
		pids[i]=0;
	for(i=0;i<w;i++){
		pid=fork();
		if(pid<=0){
			break;
		}
		else
			pids[i]=pid;
	}
	if(!pid){	
		free(pids);	
		Worker(l,1);	
	}
	
 	return pids;
}

void createWorker(pid_t pid,int l,int w,char **dirs,pid_t *pids,int *writefd,int *readfd,int bool,char *input)
{
	int *indexes,pidindex,i=0,value;
	char *basepath="tmp/fifo",buff[100];
	pid_t childpid;
	
	indexes=malloc(l*sizeof(int));
	for(i=0;i<w;i++)
		indexes[i]=-1;
	char *temp=malloc(((l+1)*MAXDIR)*sizeof(char));
	memset(temp,'\0',(l+1)*MAXDIR);
	pidindex=findPids(pid,w,pids);
	getDirFromPid(l,w,pidindex,indexes);
	assignDirs(indexes,l,dirs,temp);				//put all the dirs in a buffer (every line has a dir)
	
	
	free(indexes);
	sprintf(buff,"log/Worker_%lu.txt",(long)pids[pidindex]);
	unlink(buff);
	memset(buff,'\0',100);
	
	i=0;
	
	childpid=fork();
	if(childpid>0){
		create_npipe(childpid,pid);
		pids[pidindex]=childpid;
		sprintf(buff,"log/Worker_%lu.txt",(long)childpid);		//create logfile
		int fd=open(buff,O_RDWR | O_CREAT,PERMS);				//for the new worker
		close(fd);												//we wont use it now so we close it
		writefd[pidindex]=openFifoWriteJE(basepath,(unsigned long)childpid);			//open the new pipes
		readfd[pidindex]=openFifoReadJE(basepath,(unsigned long)childpid);
		if(write(writefd[pidindex],temp,strlen(temp))<0)					//send the dirs to worker
			perror("write error");
		while(!flag1)
			sleep(1);
		flag1=0;
		if(!bool && input!=NULL)					//give worker the query-if he has one-.
			if(write(writefd[pidindex],input,1000)<0)
				perror("write error");
		free(temp);
	}
	else{
		free(temp);
		Worker(l,bool);
	}
	
}




int findPids(pid_t pid,int w,pid_t *pids)
{
	int i;
	
	for(i=0;i<w;i++)
		if(pids[i]==pid)
			return i; 			//get the index of pid at array pids
			
	return -1;			//error no pid was found (pid should be at array pids)
	
}




