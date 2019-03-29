OBJS=sock.o main.o request.o command.o threadHTTP.o 
SRCS=$(OBJS:.o=.c)
FLAGS=-Wno-int-to-pointer-cast -Wno-pointer-to-int-cast
myhttpd: $(OBJS)
	gcc -o myhttpd -pthread  $(OBJS) $(FLAGS)
	
main.o:main.c defs.h types.h
	gcc -c main.c $(FLAGS)
sock.o:sock.c defs.h types.h
	gcc -c sock.c $(FLAGS)
request.o:request.c defs.h types.h
	gcc -c request.c $(FLAGS)
command.o:command.c defs.h types.h
	gcc -c command.c $(FLAGS)
threadHTTP.o:threadHTTP.c defs.h types.h
	gcc -c threadHTTP.c $(FLAGS)
clean: $(OBJS) 
	rm $(OBJS) myhttpd
