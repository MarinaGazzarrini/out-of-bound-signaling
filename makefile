CC = gcc
FLAGS = -L.
TFLAGS = -lpthread

.PHONY: all test clean

all: permessi myclient myserver sup 


myserver: server.c libmyf.a
	$(CC) server.c -o $@ $(FLAGS) -lmyf $(TFLAGS)


myclient: client.c
	$(CC) client.c -o $@


sup: supervisor.c libmyf.a
	$(CC) supervisor.c -o $@ $(FLAGS) -lmyf


libmyf.a: lib.o abr.o 
	ar rvs $@ lib.o abr.o


myabr.o: abr.c abr.c mylib.o
	$(CC) abr.c -c -o $@


mylib.o: lib.c lib.h
	$(CC) lib.c -c -o $@


permessi:
	@chmod 777 server.c
	@chmod 777 test.sh
	@chmod 777 misura.sh


test: 
	bash test.sh


clean: 
	rm -f *.o OOB-server-* *a sup myclient myserver



