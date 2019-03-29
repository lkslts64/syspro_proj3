OBJS=sock.o main.o request.o command.o crawler.o namedpipes.o createworkers.o navigator.o queriesJE.o queriesWorker.o trienode.o workers.o jobExecutor.o file.o  log.o
SRCS=$(OBJS:.o=.c)
FLAGS=-Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -g
mycrawler: $(OBJS)
	gcc  -o  mycrawler -pthread  $(OBJS) $(FLAGS)
	
main.o:main.c defs.h types.h
	gcc -c main.c $(FLAGS)
sock.o:sock.c defs.h types.h
	gcc -c sock.c $(FLAGS)
request.o:request.c defs.h types.h
	gcc -c request.c $(FLAGS)
command.o:command.c defs.h types.h
	gcc -c command.c $(FLAGS)
crawler.o:crawler.c defs.h types.h
	gcc -c crawler.c $(FLAGS)
namedpipes.o:namedpipes.c defs.h
	gcc -c namedpipes.c
createworkers.o:createworkers.c defs.h
	gcc -c createworkers.c
navigator.o:navigator.c defs.h
	gcc -c navigator.c
queriesJE.o:queriesJE.c defs.h
	gcc -c queriesJE.c
queriesWorker.o:queriesWorker.c defs.h
	gcc -c queriesWorker.c
trienode.o:trienode.c trienode.h defs.h
	gcc -c trienode.c
workers.o:workers.c defs.h trienode.h
	gcc -c workers.c
jobExecutor.o:jobExecutor.c defs.h
	gcc -c jobExecutor.c
file.o:file.c defs.h
	gcc -c file.c
log.o:log.c defs.h
	gcc -c log.c
clean: $(OBJS) 
	rm $(OBJS) mycrawler
