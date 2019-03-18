CC = gcc
CFLAGS = -Wall -std=gnu11

all: compile clean

compile:
	$(CC) -c main.c $(CFLAGS)
	$(CC) main.o -o main $(CFLAGS) -lpthread -lm

times:
	echo "" > Times.txt
	./main 1 baboon.pgm edge_filter.txt baboon1a.pgm >> Times.txt
	./main 1 baboon.pgm big_filter.txt baboon1b.pgm >> Times.txt
	./main 2 baboon.pgm edge_filter.txt baboon1a.pgm >> Times.txt
	./main 2 baboon.pgm big_filter.txt baboon1b.pgm >> Times.txt
	./main 4 baboon.pgm edge_filter.txt baboon1a.pgm >> Times.txt
	./main 4 baboon.pgm big_filter.txt baboon1b.pgm >> Times.txt
	./main 8 baboon.pgm edge_filter.txt baboon1a.pgm >> Times.txt
	./main 8 baboon.pgm big_filter.txt baboon1b.pgm >> Times.txt

clean:
	rm -f *.o