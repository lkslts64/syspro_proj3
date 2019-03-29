#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <poll.h>
#include <errno.h>
#include "defs.h"
#define MAXDIR 255

volatile sig_atomic_t flag1=0;


extern pid_t *pids;
extern int l,ln,*writefd,*readfd,w;
extern char **dirs,**buffer;
extern struct pollfd *fds,*fds2;
extern long maxbuffsize;
extern FILE *fp;

void sigchldInterrupt(int signo)
{
	
	pid_t pid;
	int status;
	while((pid=waitpid(-1,&status,WNOHANG | WUNTRACED))>0);
	
}


void sigusr1Interrupt(int signo)
{
	flag1=1;
}



int jobExecutor(void)			
{
	char **buffsize,*basepath="tmp/fifo";
	int i=0,index,res=0,pidindex,*indexes,value,j;
	FILE *fp;
	pid_t pid;
	
	static struct sigaction act0;
	act0.sa_handler=sigchldInterrupt;
	act0.sa_flags=SA_RESTART;
	sigfillset(&(act0.sa_mask));
	sigaction(SIGCHLD,&act0,NULL);
	
	
	static struct sigaction act2;
	act2.sa_handler=SIG_IGN;
	sigfillset(&(act2.sa_mask));
	sigaction(SIGPIPE,&act2,NULL);			//ignore SIGPIPE - we will receive this only if worker terminates abruptly 
											//we dont want JE to termiante too.
	
	static struct sigaction act1;
	act1.sa_handler=sigusr1Interrupt;
	sigfillset(&(act1.sa_mask));
	sigaction(SIGUSR1,&act1,NULL);
	
	
	
	fp=fopen("docfile","r");
	
	
	dirs=findDirs(fp,&ln);
	l=dirStats(dirs,ln);
	if(w>l)	w=l;			//if num0fWorkers > num0fDirs ,make numofworkers=numofdirs
	pids=createWorkers(w,ln);
	createLog(w,pids);
	create_npipesJE(pids,w);
	//open pipes
	readfd=malloc(w*sizeof(int));
	writefd=malloc(w*sizeof(int));
	for(i=0;i<w;i++)
		writefd[i]=openFifoWriteJE(basepath,(unsigned long)pids[i]);
	for(i=0;i<w;i++)
		readfd[i]=openFifoReadJE(basepath,(unsigned long)pids[i]);
		
	fds=malloc(w*sizeof(struct pollfd));
	for(i=0;i<w;i++)
		fds[i].fd=writefd[i];
	fds2=malloc(w*sizeof(struct pollfd));
	for(i=0;i<w;i++)
		fds2[i].fd=readfd[i];
	
	indexes=malloc(l*sizeof(int));
	for(i=0;i<l;i++)
		indexes[i]=-1;
	char *buff=malloc(((l+1)*MAXDIR)*sizeof(char));
	memset(buff,'\0',(l+1)*MAXDIR);
	for(i=0;i<w;i++){
		getDirFromPid(l,w,i,indexes);
		assignDirs(indexes,l,dirs,buff);
		if(write(writefd[i],buff,strlen(buff))<0){
			if(errno==EPIPE){								//we ignore SIGPIPE so if errno=EPIPE then worker has terminated (abruptly).
				printf("YEA\n");
				createWorker(pids[i],ln,w,dirs,pids,writefd,readfd,1,NULL);		//create a new worker replacing the dead .
			}
			else
				perror("error write");
		}
		while(!flag1)
			sleep(1);
		flag1=0;
		memset(buff,'\0',(l+1)*MAXDIR);
		for(j=0;j<l;j++)
			indexes[j]=-1;
	}
	
	free(indexes);
	free(buff);
	buffsize=malloc(w*sizeof(char*));
	for(i=0;i<w;i++){
		buffsize[i]=malloc(100*sizeof(char));
		memset(buffsize[i],'\0',100);
	}
	maxbuffsize=getMaxBuffsize(readfd,buffsize,w,writefd);
	for(i=0;i<w;i++)		//we dont want it anymore , we got the max
			free(buffsize[i]);
	free(buffsize);
	buffer=malloc(w*sizeof(char**));
	for(i=0;i<w;i++){
		buffer[i]=malloc(maxbuffsize*sizeof(char));
		memset(buffer[i],'\0',maxbuffsize);
	}
	//navigateJE(readfd,writefd,buffer,w,maxbuffsize,pids,l,dirs);	
		
		
		
		
		
	/*act0.sa_handler=SIG_DFL;
	sigaction(SIGCHLD,&act0,NULL);
	while((pid=wait(NULL))>0);		
	
	//REMEMBER TO FREE THIS WHEN WE GET SHUTDOWN ON CRAWLER
	
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
	free(buffer);*/
	return 0;
	
}





void getDirFromPid(int l,int w,int pidindex,int *indexes)	//pindex=pids[x] where x=process_pid_that_got_killed or terminated
{
	int i=0,k=0,div,mod;
	
	mod=l%w;
	div=l/w;
	while(i<div){
		indexes[i]=pidindex+k;
		k+=w;
		i++;
		
	}
	if(mod>=(pidindex+1))			//if there is a last dir
		indexes[i]=pidindex+k;
}


void assignDirs(int *indexes,int l,char **dirs,char *buff)
{
	int i=0;
	
	while(i<l && indexes[i]!=-1){
		strcat(buff,dirs[indexes[i]]);
		strcat(buff,"\n");
		i++;		
	}
}




//find the dirs of docfile
char ** findDirs(FILE *fp,int *linecount)	
{
	char c,**buffer,**buffer2;
	int i,j,ln=0,ml=0,count=0;			//ml=maxline , ln=linecount
	
	while((c=fgetc(fp))!=EOF){
		count++;
		if(c=='\n'){
			ln++;
			if(count>ml)
				ml=count;
			count=0;
		}
	}
	rewind(fp);
	buffer=malloc(ln*sizeof(char*));		//alocate memory to store docfile
	for(i=0;i<ln;i++)						//we should free when 
		buffer[i]=NULL;						//we are done
		
	//buffer2=malloc(ln*sizeof(char*));		//alocate memory to store dirs	
	//for(i=0;i<ln;i++)						//we should free when we are done 
		//buffer2[i]=malloc(ml*sizeof(char));
	//for(i=0;i<ln;i++)
		//memset(buffer2[i],'\0',ml);
	char *temp=malloc(ml*sizeof(char));		//that will be passed to workers
	memset(temp,'\0',ml);
	size_t b=0;
	int n;
	for(i=0;i<ln;i++){
		n=getline(&buffer[i],&b,fp);
		//findDir(buffer[i],n,temp);
		//storeDir(buffer2,temp,ln);
		//memset(temp,'\0',ml);
	}
	
	free(temp);
	//for(i=0;i<ln;i++)
		//free(buffer[i]);
	//free(buffer);
	(*linecount)=ln;
	return buffer;
		
}
//finds the dir on a particular line of docfile	
void findDir(char *buff,int n,char *temp)
{
	int i,flag=0,count=0;
	char *first;
	

	for(i=n-1;i>=0;i--){
		if(buff[i]=='/')
			break;
		else
			count++;
		
	}
	first=buff+i+1;
	strncpy(temp,first,count-1);
	temp[count]='\0';
}
//stores dir who was founded in findDir() in a buffer (all dirs at buffer
//are different).
void storeDir(char **buff,char *temp,int ln)
{
	int i;
	
	for(i=0;i<ln;i++){
		if(!strcmp(buff[i],"")){		//check if string is empty
			strcpy(buff[i],temp);
		}
		if(!strcmp(buff[i],temp))
			break;
	}	
}
int dirStats(char **buff,int ln)
{
	int i=0;
	while((i<ln) && strcmp(buff[i],"")!=0){
		i++;
	}
	return i;
	
}




