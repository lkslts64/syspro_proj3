#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include "defs.h"
#define PERMS 0666


void createLog(int w,pid_t *pids)
{
	int i,fd;
	char buff[100];
	
	memset(buff,'\0',100);
	mkdir("log",0755);
	
	for(i=0;i<w;i++){
		sprintf(buff,"log/Worker_%lu.txt",(long)pids[i]);
		fd=open(buff,O_RDWR | O_CREAT,PERMS);
		close(fd);
	}	
}





void logInit(char *keyword,char *queryType,int logfd)
{
	struct tm *timeinfo;
	time_t currtime;
	char buff[1000];
	
	
	memset(buff,'\0',1000);
	time(&currtime);
	timeinfo=localtime(&currtime);
	if(!strcmp(queryType,"NOsearch"))
		sprintf(buff,"%d-%d-%d %d:%d:%d : search : %s : Not found\n",(timeinfo->tm_year+1900),timeinfo->tm_mon,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec,keyword);
	else if(!strcmp(queryType,"wc"))
			sprintf(buff,"%d-%d-%d %d:%d:%d : %s : %s",(timeinfo->tm_year+1900),timeinfo->tm_mon,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec,queryType,keyword);
	else
		sprintf(buff,"%d-%d-%d %d:%d:%d : %s : %s :",(timeinfo->tm_year+1900),timeinfo->tm_mon,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec,queryType,keyword);
	if(write(logfd,buff,strlen(buff))<0)
		perror("write error loginit");
		
	
}
