CC = gcc
CFLAGS = -Wall -std=gnu11

all: compile clean

compile:
	$(CC) -c server.c $(CFLAGS)
	$(CC) server.o -o server $(CFLAGS) -lpthread
	$(CC) -c client.c $(CFLAGS)
	$(CC) client.o -o client $(CFLAGS) -lpthread

clean:
	rm -f *.o