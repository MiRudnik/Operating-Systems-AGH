CC = gcc
CFLAGS = -Wall -std=gnu11

all: static shared clean

static: blockLibrary.c
	#Compile
	$(CC) -c blockLibrary.c -o blockLibrary.o $(CFLAGS)

	#Build
	# r - insert with replacement, c - create new archive, s - write an index
	ar rcs libblockLibrary.a blockLibrary.o

shared: blockLibrary.c
	#Compile
	$(CC) $(CFLAGS) -fPIC -c blockLibrary.c -o blockLibrary.o
	#Build
	$(CC) $(CFLAGS) -fPIC -shared -o libblockLibrary.so blockLibrary.o


clean:
	rm -f *.o