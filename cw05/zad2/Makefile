CC = gcc
CFLAGS = -Wall -std=gnu11

all: compile clean

compile:
	$(CC) -c main.c $(CFLAGS)
	$(CC) main.o -o main $(CFLAGS)

	$(CC) -c master.c $(CFLAGS)
	$(CC) master.o -o master $(CFLAGS)

	$(CC) -c slave.c $(CFLAGS)
	$(CC) slave.o -o slave $(CFLAGS)

clean:
	rm -f *.o