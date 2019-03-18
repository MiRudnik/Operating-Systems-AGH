CC = gcc
CFLAGS = -Wall -std=gnu11

all: compile clean

compile:
	$(CC) -c main.c $(CFLAGS)
	$(CC) main.o -o main $(CFLAGS)
	$(CC) -c doChild.c $(CFLAGS)
	$(CC) doChild.o -o doChild $(CFLAGS)
	
clean:
	rm -f *.o