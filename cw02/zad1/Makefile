CC = gcc
CFLAGS = -Wall -std=gnu11

all: compile test clean

compile:
	$(CC) -c main.c $(CFLAGS)
	$(CC) main.o -o main $(CFLAGS)

test:
	echo "" > wyniki.txt
	make test4
	echo "" >> wyniki.txt
	make test512
	echo "" >> wyniki.txt
	make test4096
	echo "" >> wyniki.txt
	make test8192

test4:
	echo "*****4bytes*****" >> wyniki.txt
	./main generate test 4000 4
	./main copy test copySys 4000 4 sys >> wyniki.txt
	./main copy test copyLib 4000 4 lib >> wyniki.txt
	./main sort copySys 4000 4 sys >> wyniki.txt
	./main sort copyLib 4000 4 lib >> wyniki.txt
	./main generate test 5000 4
	./main copy test copySys 5000 4 sys >> wyniki.txt
	./main copy test copyLib 5000 4 lib >> wyniki.txt
	./main sort copySys 5000 4 sys >> wyniki.txt
	./main sort copyLib 5000 4 lib >> wyniki.txt

test512:
	echo "*****512bytes*****" >> wyniki.txt
	./main generate test 4000 512
	./main copy test copySys 4000 512 sys >> wyniki.txt
	./main copy test copyLib 4000 512 lib >> wyniki.txt
	./main sort copySys 4000 512 sys >> wyniki.txt
	./main sort copyLib 4000 512 lib >> wyniki.txt
	./main generate test 5000 512
	./main copy test copySys 5000 512 sys >> wyniki.txt
	./main copy test copyLib 5000 512 lib >> wyniki.txt
	./main sort copySys 5000 512 sys >> wyniki.txt
	./main sort copyLib 5000 512 lib >> wyniki.txt

test4096:
	echo "*****4096bytes*****" >> wyniki.txt
	./main generate test 4000 4096
	./main copy test copySys 4000 4096 sys >> wyniki.txt
	./main copy test copyLib 4000 4096 lib >> wyniki.txt
	./main sort copySys 4000 4096 sys >> wyniki.txt
	./main sort copyLib 4000 4096 lib >> wyniki.txt
	./main generate test 5000 4096
	./main copy test copySys 5000 4096 sys >> wyniki.txt
	./main copy test copyLib 5000 4096 lib >> wyniki.txt
	./main sort copySys 5000 4096 sys >> wyniki.txt
	./main sort copyLib 5000 4096 lib >> wyniki.txt

test8192:
	echo "*****8192bytes*****" >> wyniki.txt
	./main generate test 4000 8192
	./main copy test copySys 4000 8192 sys >> wyniki.txt
	./main copy test copyLib 4000 8192 lib >> wyniki.txt
	./main sort copySys 4000 8192 sys >> wyniki.txt
	./main sort copyLib 4000 8192 lib >> wyniki.txt
	./main generate test 5000 8192
	./main copy test copySys 5000 8192 sys >> wyniki.txt
	./main copy test copyLib 5000 8192 lib >> wyniki.txt
	./main sort copySys 5000 8192 sys >> wyniki.txt
	./main sort copyLib 5000 8192 lib >> wyniki.txt

clean:
	rm -f *.o