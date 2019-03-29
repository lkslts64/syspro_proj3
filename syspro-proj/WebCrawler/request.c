#include <stdio.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/types.h>
#include <dirent.h>
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
#include <limits.h>
#include "types.h"
#include "defs.h"
extern int port;
extern char *hostip,*savedir;
extern pthread_mutex_t mtx2;		//mutex for metadata (number of pages download and bytes downloaded)
extern struct metadata meta;
extern pool_t pool;
//send request - get respone - return html text
char * sendRequest(char *url,int sock)
{
	char request[2046];
	char *token1,*token2,*saveptr,*delimeter="/";
	
	char file[1024],webdir[256];
	long bytes=10000,bytes2;
	DIR *dir;
	int i,last,count=0,n,length=6,fdoc,counter2=0;
	
	bytes2=bytes;
	//bytes+=50;
	bytes++;
	//char *response1=malloc(bytes*sizeof(char));
	//tmp=malloc(10000*sizeof(char));
	//memset(tmp,'\0',10000);
	//memset(response1,'\0',bytes);
	
	memset(file,'\0',1024);
	memset(request,'\0',2046);
	memset(webdir,'\0',256);
	last=strlen(url)-1;
	i=last;
	int j=0;		
	while(count<2){				//get file (/site0/page0_2342342.html for example) from URL.
		if(url[i]=='/'){
			count++;
		}
		i--;
	}
	i++;
	strcpy(file,url+i);
	char file2[1024];
	memset(file2,'\0',1024);
	strcpy(file2,file);
	sprintf(file2,"%s%s",savedir,file);
	//create request.
	//the request has only a Host header except the GET . (Host: is essential)
	
	
	//char *omg="49044";
	//int omg2=atoi(omg);
	//printf("%d\n",omg2);
	
	sprintf(request,"GET %s HTTP/1.1\r\nHost: %s:%d",file,hostip,port);
	if(write(sock,request,strlen(request))<0)
		perror("error write");
		
		
	//char *response2;
	/*char tmp[10000];
	memset(tmp,'\0',10000);
	printf("BEFORE READING\n");
	printf("tmp is %p and response is %p\n",tmp,response1);
	while((n=read(sock,tmp,bytes2))>0){
		bytes+=bytes2;
		strcat(response1,tmp);
		//printf("n is %d\n",n);
		char *response2=realloc(response1,bytes*sizeof(char));
		if(response2!=NULL)
			response1=response2;
		else
			printf("ERROR REALLOC\n");
		
		memset(tmp,'\0',bytes2);
		printf("AAA\n");
		//response[strlen(response)]='\0';
		//if(n<10000)
			//break;
	}
	response1[strlen(response1)]='\0';
	//printf("%s\n",response);
	printf("AFTER READING\n");*/
	
	//free(tmp);
	char *len;
	char temp[1];
	
	char tmp[8000];
	memset(tmp,'\0',8000);
	
	while((n=read(sock,temp,1))>0){
		strcat(tmp,temp);
		counter2++;
		if(counter2>=8000){
			printf("Crawler cant handle HTTP headers bigger than 8KB . We are sorry..\n");
			return NULL;
		}
		
		len=strstr(tmp,"\n\n");
		if(len!=NULL)
			break;
		
		
	
	}
	len=strstr(tmp,"Content-Length: ");
	//char valid[1024];
	char *valid=malloc(1024*sizeof(char));
	memset(valid,'\0',1024);
	if(len!=NULL){
		len+=16;
		i=0;
		while(len[i]!='\n'){
			valid[i]=len[i];
			i++;
		}
		//char string1[]="132432";
		char *end=NULL;
		length=strToInt(valid);
			
	}
	else{
		printf("Content-Length Header send . Server made m	istake.\n");
		return NULL;
	}
	printf("length is %d \n",length);
	char *htmltext=malloc((length+50) * sizeof(char));
	memset(htmltext,'\0',length+50);
	char *response=malloc((length+50+8000)*sizeof(char));
	memset(response,'\0',length+50+8000);
	strcat(response,tmp);
	while((n=read(sock,htmltext,length+50))>0){
		printf("1");

		
		strcat(response,htmltext);
		memset(htmltext,'\0',length+50);
	}
	//printf("%s\n",response);
	
	
	free(htmltext);
	memset(valid,'\0',1024);
	char *nline;
	nline=strchr(response,'\n');
	strncpy(valid,response,(size_t)(nline-response));
	if(strcmp(valid,"HTTP/1.1 200 OK\r")!=0)					//validate response
		return NULL;
		
	
	
	free(valid);
	pthread_mutex_lock(&mtx2);
	meta.bytes+=length;
	meta.pages++;
	pthread_mutex_unlock(&mtx2);
	
	token1=strtok_r(file,delimeter,&saveptr);			//has dir (for example ->site0)
	token2=strtok_r(NULL,delimeter,&saveptr);			//has html file
	sprintf(webdir,"%s%s",savedir,token1);				//have absolute path (for example k24/save_dir/site0)
	dir=opendir(webdir);	
	if(dir==NULL && errno==ENOENT){
		mkdir(webdir,0777);
		if((fdoc=open("docfile",O_CREAT | O_RDWR | O_APPEND,0644))<0)
			perror("open docfile error");
		strcat(webdir,"\n");
		if(write(fdoc,webdir,strlen(webdir))<0)
			perror("write error");
		close(fdoc);
	}
	int fd;
	if((fd=open(file2,O_CREAT | O_RDWR,0644))<0)
		perror("ERROR at open file\n");
	char *text;
	
	text=strstr(response,"\n\n");
	text+=2;
	if(write(fd,text,strlen(text))<0)		//write to new file.
		perror("ERROR write\n");
	
	close(fd);
	closedir(dir);
	return response;
	
	
}
//search for links.
void findLinks(char *response)
{
	char *startlink,*endlink,link[1024],url[2083],*text;
	memset(link,'\0',1024);
	memset(url,'\0',2083);
	text=strstr(response,"\n\n");
	text+=2;
	startlink=strstr(text,"<a href=");
	while(startlink!=NULL){
		startlink+=8;
		endlink=strchr(startlink,'>');
		strncpy(link,startlink,(size_t)(endlink-startlink));
		sprintf(url,"http://%s:%d%s",hostip,port,link);
		place(url);
		startlink=strstr(endlink,"<a href=");
		
		memset(link,'\0',1024);
		memset(url,'\0',2083);
		
	}
	
}
int strToInt( char *text)
{
  int n = 0, sign = 1;
  switch (*text) {
    case '-': sign = -1;
    case '+': ++text;
  }
  for (; isdigit(*text); ++text) {

	n *= 10; 

	n += *text - '0';

	}
  return n * sign;
}
