#include <stdio.h>
#include <sys/types.h>
/* sockets */
#include <sys/socket.h>
/* sockets */
#include <netinet/in.h>
/* internet sockets */
#include <unistd.h>
/* read, write, close */
#include <netdb.h>
/* gethostbyaddr */
#include <stdlib.h>
/* exit */
#include <string.h>
/* strlen */



void main(int argc, char *argv[]) {
int
port, sock, i;
char
buf[256];
struct sockaddr_in server;
struct sockaddr *serverptr = (struct sockaddr*)&server;
struct hostent *rem;
if (argc != 3) {
	printf("Please give host name and port number\n");
	exit(1);}
/* Create socket */
if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	perror("socket");
/* Find server address */
if ((rem = gethostbyname(argv[1])) == NULL) {
	herror("gethostbyname"); exit(1);
}port = atoi(argv[2]); /*Convert port number to integer*/
server.sin_family = AF_INET;
/* Internet domain */
memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
server.sin_port = htons(port);
/* Server port */
/* Initiate connection */
if (connect(sock, serverptr, sizeof(server)) < 0)
	perror("connect");
printf("Connecting to %s port %d\n", argv[1], port);
do {
	printf("Give input string: ");
	fgets(buf, sizeof(buf), stdin); /* Read from stdin*/
	for(i=0; buf[i] != '\0'; i++) { /* For every char */
	/* Send i-th character */
	if (write(sock, buf,strlen(buf)) < 0)
		perror("write");
	/* receive i-th character transformed */
	if (read(sock, buf,1000) < 0)
		perror("read");
	}
	printf("Received string: %s", buf);
} while (strcmp(buf, "END\n") != 0); /* Finish on "end" */
close(sock);
}
