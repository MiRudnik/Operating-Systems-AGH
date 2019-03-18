CC = gcc
CFLAGS = -Wall -std=gnu11

all: compile clean

compile:
	$(CC) -c main.c $(CFLAGS)
	$(CC) main.o -o main $(CFLAGS)
	$(CC) -c endlessLoop.c $(CFLAGS)
	$(CC) endlessLoop.o -o endlessLoop $(CFLAGS)

clean:
	rm -f *.o