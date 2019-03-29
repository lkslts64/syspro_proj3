#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
//extern int erno;
#include "defs.h"
#define PERMS 0666
//create named pipe if JE gets a SIGCHLD and delete the named pipes that dead worker had.
void create_npipe(pid_t pid,pid_t deadpid)		
{
	char *basepath="tmp/fifo";
	char path1[50],path2[50],path3[50],path4[50];
	
	sprintf(path3,"%s_1_%lu",basepath,(long)deadpid);		//deletion of named pipes of dead worker
	sprintf(path4,"%s_2_%lu",basepath,(long)deadpid);
	unlink(path3);
	unlink(path4);
	
	sprintf(path1,"%s_1_%lu",basepath,(long)pid);		//this is for reading
	sprintf(path2,"%s_2_%lu",basepath,(long)pid);		//this is for writing
	if((mkfifo(path1,PERMS)<0) && (errno!=EEXIST))
		perror("cant create fifo");
	if((mkfifo(path2,PERMS)<0) && (errno!=EEXIST)){
		unlink(path1);
		perror("cant create fifo");
	}
}
//jobExecutor creating pipes
void create_npipesJE(int *pids,int w)	
{
	char *basepath="tmp/fifo";
	char path1[50],path2[50];
	int i;
	
	for(i=0;i<w;i++){
		sprintf(path1,"%s_1_%lu",basepath,(unsigned long)pids[i]);
		sprintf(path2,"%s_2_%lu",basepath,(unsigned long)pids[i]);
		if((mkfifo(path1,PERMS)<0) && (errno!=EEXIST))
			perror("cant create fifo");
		if((mkfifo(path2,PERMS)<0) && (errno!=EEXIST)){
			unlink(path1);
			perror("cant create fifo");
		}
	}
}
//open name pipe that Worker reads data
int openFifoReadWorker(char *basepath)
{
	char path[50];
	int readfd;
	sprintf(path,"%s_2_%lu",basepath,(unsigned long)getpid());
	while((readfd=open(path,O_RDONLY))<0){};
	return readfd;
}
//open name pipe that Worker writes data.
int openFifoWriteWorker(char *basepath)
{
	char path[50];
	int writefd;
	sprintf(path,"%s_1_%lu",basepath,(unsigned long)getpid());
	while((writefd=open(path,O_WRONLY))<0){};
	return writefd;
}
//open name pipe that jobExecutor reads data ( NONBLOCK) 
int openFifoReadJE(char *basepath,unsigned long pid)
{
	char path[50];
	int readfd;
	sprintf(path,"%s_1_%lu",basepath,pid);
	//sprintf(path,"%s_1_%d",BASEPATH,getpid());			
	if((readfd=open(path,O_RDONLY | O_NONBLOCK))<0)
		perror("cant openFifoReadJE fifo to read");
	return readfd;
}
//open name pipe that jobExecutor writes data . (NONBLOCK)
int openFifoWriteJE(char *basepath,unsigned long pid)
{
	char path[50];
	int writefd;
	sprintf(path,"%s_2_%lu",basepath,pid);
	//sprintf(path,"%s_1_%d",BASEPATH,getpid());
	if((writefd=open(path,O_WRONLY))<0){
		printf("error at open named pipe openFifoWriteJ\n");
		perror("cant openFifoWriteJE fifo to write");
	}
	//if((writefd=open(path,O_WRONLY))<0)
	return writefd;
}
