CC = gcc
CFLAGS = -Wall

all: main

main: main.o cethreads.o
	$(CC) $(CFLAGS) -o main main.o cethreads.o

main.o: main.c cethreads.h
	$(CC) $(CFLAGS) -c main.c

cethreads.o: cethreads.c cethreads.h
	$(CC) $(CFLAGS) -c cethreads.c

clean:
	rm -f *.o main