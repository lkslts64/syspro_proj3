#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include "types.h"
#include "defs.h"


int main(void){

	char *string="432342";
	
	int length=atoi(string);
	printf("length is %d \n",length);
	
	return 0;
}
