CC = gcc
CFLAGS = -Wall -std=gnu11

all: compile test clean

compile:
	$(CC) -c blockLibrary.c $(CFLAGS)
	$(CC) -c main.c $(CFLAGS)
	$(CC) main.o blockLibrary.o -o main $(CFLAGS)

test:
	echo "" > raport2.txt
	./main dynamic 5000 5000 search 2 remove 200 add 200 remove_and_add 200 >> raport2.txt
	./main static 5000 5000 search 2 remove 200 add 200 remove_and_add 200 >> raport2.txt

clean:
	rm -f *.o