CC = gcc
CFLAGS = -Wall -std=gnu11

all: compile clean

compile:
	$(CC) -c main_a.c $(CFLAGS)
	$(CC) main_a.o -o main_a $(CFLAGS)
	$(CC) -c main_b.c $(CFLAGS)
	$(CC) main_b.o -o main_b $(CFLAGS)

clean:
	rm -f *.o