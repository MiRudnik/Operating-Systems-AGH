CC = gcc
CFLAGS = -Wall -std=gnu11

all: compile clean

compile:
	$(CC) -c client.c $(CFLAGS)
	$(CC) client.o -o client $(CFLAGS) -lpthread -lrt
	$(CC) -c barber.c $(CFLAGS)
	$(CC) barber.o -o barber $(CFLAGS) -lpthread -lrt


clean:
	rm -f *.o