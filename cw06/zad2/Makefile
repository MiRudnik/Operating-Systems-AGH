CC = gcc
CFLAGS = -Wall -std=gnu11

all: compile clean

compile:
	$(CC) -c client.c $(CFLAGS)
	$(CC) client.o -o client $(CFLAGS) -lrt
	$(CC) -c server.c $(CFLAGS)
	$(CC) server.o -o server $(CFLAGS) -lrt


clean:
	rm -f *.o