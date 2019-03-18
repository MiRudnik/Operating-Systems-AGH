CC = gcc
CFLAGS = -Wall -std=gnu11

all: compile clean

compile:
	$(CC) -c main.c $(CFLAGS)
	$(CC) main.o -o main $(CFLAGS)

clean:
	rm -f *.o

test:
	./main ~/Dokumenty/ '=' 2018-03-14 nftw
	./main ~/Dokumenty/ '>' 2010-01-01 nftw	
	./main ~/Dokumenty/RudnikMichal/cw02 '>' 2010-01-01 stat