#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <poll.h>
#include <sys/poll.h>
#include <sys/time.h>
#include "defs.h"

extern pid_t *pids;
extern int l,ln,*writefd,*readfd,w;
extern char **dirs,**buffer;
extern struct pollfd *fds,*fds2;
extern long maxbuffsize;


extern volatile sig_atomic_t flag2;
//jobExecutor should know how much memory to allocate for the result of the queries
//This amount of memory is the return value of this function.
//read in NONBLOCK mode. Read until receivs w answers from workers.
int getMaxBuffsize(int *readfd,char **buffsize,int w,int *writefd)
{
	int i,n,count=0;
	long max=0,temp;
	
	while(count<w){
		//printf("Maxbuff function\n");
		if(polling2(1,NULL)>0){	//poll for readfd
			for(i=0;i<w;i++){
				if((n=read(readfd[i],buffsize[i],100))>0){
					count++;
					temp=atol(buffsize[i]);
					if(temp>max)
						max=temp;
					memset(buffsize[i],'\0',100);
				}
			}
		}
	}
	printf("Files Loaded from all Workers\n");
	return max;	
}

/*void ReceiveMaxcount(int *readfd,int w,long buffsize,int *writefd)
{
	int i,n,count=0,max=0,temp;
	char *token,temp2[512];
	char *delim=" ",buffer[1000];
	
	memset(buffer,'\0',1000);
	
	while(count<w){		//read until JE get w answers(w is noworkers)
		if(polling2(fds2,w,readfd,writefd,0,input1)>0){	//poll for readfd
			for(i=0;i<w;i++){
				if((n=read(readfd[i],buffer,1000))>0){
					count++;
					token=strtok(buffer,delim);
					temp=atoi(token);
					if(temp>=max){
						token=strtok(NULL,delim);
						if(temp==max && max>0){	//condition to print the 
							if(strcmp(token,temp2)<0)//the smaller alphabetical
								strcpy(temp2,token);//full path
						}
						else
							strcpy(temp2,token);
						max=temp;
					}
					memset(buffer,'\0',1000);
				}
			}
		}
	}
	if(max>0)
		printf("%d\t%s\n",max,temp2);
	else
		printf("None worker found this word\n");
}

void ReceiveMincount(int *readfd,int w,long buffsize,int *writefd)
{
	int i,n,count=0,min,temp;
	char *token,temp2[512];
	char *delim=" ",buffer[1000];
	
	
	memset(buffer,'\0',1000);
	
	min=(unsigned int)buffsize;
	while(count<w){		//read until JE get w answers(w is noworkers)
		if(polling2(fds2,w,readfd,writefd,0,input1)>0){	//poll for readfd
			for(i=0;i<w;i++){
				if((n=read(readfd[i],buffer,1000))>0){		//if worker's fd is closed , still count++ so we can exit the loop
					count++;
					token=strtok(buffer,delim);
					temp=atoi(token);
					if(temp<=min && temp>0){
						token=strtok(NULL,delim);
						if(temp==min && min<(unsigned int)buffsize){	//condition to print the 
							if(strcmp(token,temp2)<0)//the smaller alphabetical
								strcpy(temp2,token);//full path
						}
						else
							strcpy(temp2,token);
						min=temp;
					}
					memset(buffer,'\0',1000);
				}
			}
		}
	}
	if(min!=(unsigned int)buffsize)
		printf("%d\t%s\n",min,temp2);
	else
		printf("None worker found this word\n");	
}

void ReceiveWc(int *readfd,int w,int *writefd)
{
	int i,n,count=0,sumwords=0,sumlines=0;
	long sumbytes=0,temp;
	char buffer[1000],*delim=" \n",*token;
	
	
	while(count<w){
		if(polling2(fds2,w,readfd,writefd,0,input1)>0){		//poll for readfd				
			for(i=0;i<w;i++){
				if((n=read(readfd[i],buffer,1000))>0){
					count++;
					token=strtok(buffer,delim);
					temp=atol(token);
					sumbytes+=temp;
					token=strtok(NULL,delim);
					temp=atoi(token);
					sumwords+=temp;
					token=strtok(NULL,delim);
					temp=atoi(token);
					sumlines+=temp;
					memset(buffer,'\0',1000);
				}
			}
		}
	}
	printf("%d\t%d\t%lu\n",sumlines,sumwords,sumbytes);
}*/

void ReceiveExit(void)
{
	int count=0,temp,i,n;
	char buffer[1000];
	
	while(count<w){
			for(i=0;i<w;i++){
				if((n=read(readfd[i],buffer,1000))>0){
					count++;
					temp=atoi(buffer);
					printf("Worker_%lu found %d strings\n",(long)pids[i],temp);
					memset(buffer,'\0',1000);
				}
			}
	} 
}




void ReceiveSearch(char *input)
{
	int i,count=0,n,length;
	time_t timer;
	struct timeval stop;
	double stopage;
	
	char *temp=malloc(maxbuffsize*sizeof(char));
	//double *seconds=malloc(w*sizeof(double));
	
	memset(temp,'\0',maxbuffsize);
	//for(i=0;i<w;i++)
		//seconds[i]=-1.0;
	while(count<w){		//read until JE get w answers(w is noworkers)
		if(polling2(0,input)>0){	//poll for readfd
			for(i=0;i<w;i++){
				while((n=read(readfd[i],temp,maxbuffsize))>0){			//we add ==0 because worker maybe has exited (read returns 0 if fd has been closed at worker)
					strcat(buffer[i],temp);
					if((temp[strlen(temp)-1]=='\n') && (temp[strlen(temp)-2]=='\n')){
						count++;						
						//gettimeofday(&stop,NULL);
						//seconds[i]=difftime(timer,initTimer);
						//seconds[i]=(double)(stop.tv_usec-start.tv_usec)/1000000.0+(stop.tv_sec-start.tv_sec);
						
					}
					memset(temp,'\0',maxbuffsize);
				}
			}
		}
		//time(&timer);
		//if((stopage=difftime(timer,initTimer))>=180)
			//break;
	}
	count=0;
	for(i=0;i<w;i++){
		//if(seconds[i]<=deadline){				//take into account deadline
			//printf("Worker with pid %lu:\n%s\n-------------------------------------------\n",(long)pids[i],buffer[i]);		//printf results from workers
			count++;
		//}
	}
	//printf("%d from %d Workers answered.\n",count,w);
	free(temp);
	//free(seconds);
}


void sendQeury(char *input)
{
	int i;
	
	if(polling(0,input)<=0)
		for(i=0;i<w;i++)
			if(write(writefd[i],input,strlen(input))<0)
				perror("write error sendquery");	
}


//Checking if writefds at JE are available . if they are not and POL_ERR bit is 1 , then a worker is dead and we need replacement.
int polling(int bool,char *input)
{
	int i,r,count=0;
	
	for(i=0;i<w;i++)
		fds[i].fd=writefd[i];

	r=poll(fds,w,0);
	if(r>0){
		for(i=0;i<w;i++){
			if((fds[i].revents & POLLERR)||(fds[i].revents & POLLNVAL)){
				printf("Creating worker\n");
				count++;
				createWorker(pids[i],l,w,dirs,pids,writefd,readfd,bool,input);
				//fds[i].revents=0;
			}
		}
	}
	return count;
}
//Checking if readfds are available at JE.if POLLHUP bit is 1 ,then a worker is dead and we need replacement
int polling2(int bool,char *input)
{
	int i,r,flag=0;
	
	for(i=0;i<w;i++){
		fds2[i].fd=readfd[i];
		fds2[i].events=POLLIN;
	}
	
	r=poll(fds2,w,100);
	if(r>0){
		for(i=0;i<w;i++){
			if((fds2[i].revents & POLLIN)){
				flag=1;
			}
			if(fds2[i].revents & POLLHUP){
				printf("Creating worker maxbuff\n");
				createWorker(pids[i],ln,w,dirs,pids,writefd,readfd,bool,input);
			}
		}
	}
	return flag;
	
	
	
}

