#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include "defs.h"
#define MAXDIR 255
#define PERMS 0666




char **loadFiles(char **buff,int l,int *counter,long *msize,char ***fnames,long *wc)		//we have all dirs stored at **buff (2d array)
{
	
	int i,dirsno=0,j,count=0,bytes;
	DIR *dir;
	FILE *fp;
	long size,maxsize=0,sum=0;
	struct dirent *direntp;
	char path[512];		//dir and filename (max 255 each)
	
	for(i=0;i<l;i++){
		if(strcmp(buff[i],"")!=0){
			dirsno++;
		}
	}
	
	for(j=0;j<dirsno;j++){			//counting number of files that worker owns
		if((dir=opendir(buff[j]))==NULL){
			fprintf(stderr, "cannot open %s \n",buff[j]);
		}
		else{
			while((direntp=readdir(dir))!=NULL){
				if((!strcmp(direntp->d_name,".")) || (!strcmp(direntp->d_name,"..")))
					continue;
				count++;	//holds the files that worker owns
				sprintf(path,"%s/%s",buff[j],direntp->d_name);
				if((fp=fopen(path,"r"))==NULL){		//open files 
					printf("fopen error because:%s",strerror(errno));
					
				}
				if(fseek(fp,0L,SEEK_END)!=0)		//count the size of file
					printf("fseek error\n");
				size=ftell(fp);		//here store size
				if(size>maxsize)
					maxsize=size;		//holds the size of files
				fseek(fp,0L,SEEK_SET);		//rewind (required)
				fclose(fp);
				memset(path,'\0',512);	
			}			
		}
		closedir(dir);
	}
	char **map;	//here we will store all the files
	map=malloc(count*sizeof(char *));
	for(i=0;i<count;i++)
		map[i]=malloc((maxsize+1)*sizeof(char));	//allocate memory for all files of worker
	for(i=0;i<count;i++)
		memset(map[i],'\0',maxsize+1);			//intialize map
		
		
	char **names;
	names=malloc(count*sizeof(char*));
	for(i=0;i<count;i++)
		names[i]=malloc(512*sizeof(char));
	for(i=0;i<count;i++)
		memset(names[i],'\0',512);
	i=0;
	for(j=0;j<dirsno;j++){			//counting number of files that worker owns
		if((dir=opendir(buff[j]))==NULL)
			fprintf(stderr, "cannot open %s \n",buff[j]);
		else{
			while((direntp=readdir(dir))!=NULL){
				if((!strcmp(direntp->d_name,".")) || (!strcmp(direntp->d_name,"..")))
					continue;
				sprintf(path,"%s/%s",buff[j],direntp->d_name);
				strcpy(names[i],path);
				fp=fopen(path,"r");
				bytes=fread(map[i],sizeof(char),maxsize,fp);
				sum+=bytes;
				//map[maxsize]='\0';
				i++;
				fclose(fp);
				memset(path,'\0',512);
			}
		}
		closedir(dir);
	}
	(*counter)=count;
	(*msize)=maxsize+1;
	(*fnames)=names;
	(*wc)=sum;
	return map;			//we should free this at end of worker
}



int seperateFiles(char **map,char **fnames,int count,int msize,int *sumlines,int *sumwords)
{
	int lines=1,offset=0,i,j=0,counter=0,maxline=0;
	char *temp;
	
	temp=malloc(msize*sizeof(char));
	memset(temp,'\0',msize);
	
	for(i=0;i<count;i++){
		while(map[i][j]!='\0'){
			if((map[i][j]=='\n' || map[i][j]==' ' || map[i][j]=='\t')) {
				if(counter){
					(*sumwords)++;
					counter=0;
					trieInsert(temp,lines,offset,i);
					memset(temp,'\0',msize);
				}
				if(map[i][j]=='\n'){
					lines++;
					if(lines>maxline)
						maxline=lines;
					offset=j;	//map[i][offset] is the next char after newline
					offset++;
				}
						
			}
			else{
				temp[counter]=map[i][j];
				counter++;
			}
			j++;
		}
		j=0;
		(*sumlines)+=lines;
		(*sumlines)--;			//because we start lines from 1
		lines=1;
		offset=0;
	}

	


	free(temp);
	return maxline;

}







