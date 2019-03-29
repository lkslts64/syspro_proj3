#define _GNU_SOURCE 
#include <stdio.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <poll.h>
#include <fcntl.h>
#include "types.h"
#include "defs.h"

extern int serverport;
extern char *rootdir;
extern pthread_mutex_t mtx2;		//mutex for metadata (number of pages served and bytes served)
extern struct metadata meta;

int getRequest(int fd)
{
	int l=8000,n,sum=0,flag2=0,count=0,len,res;
	FILE *fp;
	struct pollfd pfd;
	
	
	len=l;
	char *buff,*delimeter=" \r\n",*response,temp[8000];
	char *badresponse="You didnt send an HTTP request\n";
	
	
	memset(temp,'\0',8000);
	buff=malloc(l*sizeof(char));
	memset(buff,'\0',l);
	
	pfd.fd=fd;
	pfd.events=POLLIN;
	
	res=poll(&pfd,1,10000);
	if(!res){
		printf("WE WAITED TOO LONG\n");
		return -1;					//we waited too long...
	}
	char *buff1;
	while(pfd.revents & POLLIN){
		if(count>0){
			buff1=realloc(buff,l*sizeof(char));
			if(buff1==NULL)
				exit(0);	//exiting..
			else
				buff=buff1;
		}
		if((n=read(fd,temp,len))<0)
			perror("read");
		if(!n)
			return -1;				//client closed his socket
		strcat(buff,temp);
		memset(temp,'\0',len);
		poll(&pfd,1,0);
		if((pfd.revents & POLLHUP) || (pfd.revents & POLLRDHUP))
			printf("CLIENT CLOSED CONNECTION\n");
		l+=len;
		count++;
	}
	char *token,*saveptr,file[512];
	
	memset(file,'\0',512);
	token=strtok_r(buff,delimeter,&saveptr);
	if(token==NULL || strcmp(token,"GET"))
		goto bad;
	token=strtok_r(NULL,delimeter,&saveptr);
	if(token!=NULL)
		strcpy(file,token);
	else
		goto bad;
	token=strtok_r(NULL,delimeter,&saveptr);
	if(token==NULL || strcmp(token,"HTTP/1.1"))
		goto bad;
	while(token!=NULL){
		token=strtok_r(NULL,delimeter,&saveptr);
		if(token!=NULL && !strcmp(token,"Host:")){
			flag2=1;
			break;
		}
	}
	if(!flag2)
		goto bad;
	free(buff);
	char path[1024];
	int filefd;
	
	
	char rd[256];
	memset(rd,'\0',256);
	strcpy(rd,rootdir);
	rd[strlen(rd)-1]='\0';
	 
	memset(path,'\0',1024);
	sprintf(path,"%s%s",rd,file);
	if((filefd=open(path,O_RDONLY))<0) {
		if(errno==EACCES)
			response=createResponse(filefd,-1);
		else if(errno==ENOENT)
			response=createResponse(filefd,-2);
	}
	else{
		struct stat st;
		fstat(filefd,&st);
		int size=st.st_size;
		pthread_mutex_lock(&mtx2);		//update the metadata when found a page
		meta.pages++;
		meta.bytes+=size;
		pthread_mutex_unlock(&mtx2);
		response=createResponse(filefd,size);
		
	}
	int length=strlen(response);
	if(write(fd,response,length)<0)
		perror("write");
	free(response);
	close(filefd);
	return 0;
	
bad: if(write(fd,badresponse,strlen(badresponse))<0)
		perror("write");
	return -1;
	
}

//-if size is >0 ,then file exists and we can send it
//-if size is -1 , then we dont have permissions 
//-if size is -2 , then file doesnt exists. 
char *createResponse(int filefd,int size)
{
	
	char *buff;
	int n;
	
	
	
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	timeinfo = localtime (&rawtime);
	char timebuff[100];
	strftime(timebuff,100,"Date: %a, %e %h %G %T GMT\n",timeinfo);
	
	buff=malloc((size+5000)*sizeof(char));	//5000 for HTTP headers 
	memset(buff,'\0',size+5000);
	
	if(size>0){
		sprintf(buff,"HTTP/1.1 200 OK\r\n");
		strcat(buff,timebuff);
		strcat(buff,"Server: myhttpd/1.0.0\n");
		char buff2[100];
		sprintf(buff2,"Content-Length: %d\n",size);
		strcat(buff,buff2);
		strcat(buff,"Content-Type: text/html\nConnection: Closed\n\n");
		char *buff3=malloc((size+1)*sizeof(char));
		memset(buff3,'\0',size+1);
		while((n=read(filefd,buff3,size+1))>0);
		strcat(buff,buff3);
		free(buff3);
	}
	else if(size==-1){
		sprintf(buff,"HTTP/1.1 403 Forbidden\r\n");
		strcat(buff,timebuff);
		strcat(buff,"Server: myhttpd/1.0.0\n");
		char buff2[100];
		sprintf(buff2,"Content-Length: 124\n");
		strcat(buff,buff2);
		strcat(buff,"Content-Type: text/html\nConnection: Closed\n\n");
		strcat(buff,"<!DOCTYPE html>\n<html>\n\tTrying to access this file but dont think I can make it.\n</html>");
	}
	else if(size==-2){
		sprintf(buff,"HTTP/1.1 404 Not Found\r\n");
		strcat(buff,timebuff);
		strcat(buff,"Server: myhttpd/1.0.0\n");
		char buff2[100];
		sprintf(buff2,"Content-Length: 124\n");
		strcat(buff,buff2);
		strcat(buff,"Content-Type: text/html\nConnection: Closed\n\n");
		strcat(buff,"<!DOCTYPE html>\n<html>\n\tSorry dude, could not find this file.\n</html>");
	}
	//printf("%s\n",buff);
	return buff;
	
}
