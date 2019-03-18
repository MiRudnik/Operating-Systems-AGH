CC = gcc
CFLAGS = -Wall -std=gnu11

all: main_static main_dynamic main_shared test clean

static:
	$(CC) -c blockLibrary.c -o blockLibrary.o $(CFLAGS)
	ar rcs libBlockLibrary.a blockLibrary.o

shared:
	$(CC) $(CFLAGS) -fPIC -c blockLibrary.c -o blockLibrary.o
	$(CC) $(CFLAGS) -fPIC -shared -o libBlockLibrary.so blockLibrary.o

main_static:
	make static
	$(CC) -o main_static main.c libBlockLibrary.a


main_dynamic:
	make shared
	$(CC) -o main_dynamic main_dyn.c -ldl


main_shared:
	make shared
	$(CC) -o main_shared main.c libBlockLibrary.so


test:
	echo "" > raport3a.txt
	echo "Static libraries: " >> raport3a.txt
	echo "______________________________" >> raport3a.txt
	./main_static dynamic 5000 5000 search 2 remove 200 add 200 remove_and_add 200 >> raport3a.txt
	./main_static static 5000 5000 search 2 remove 200 add 200 remove_and_add 200 >> raport3a.txt
	echo "Dynamic libraries: " >> raport3a.txt
	echo "______________________________" >> raport3a.txt
	./main_dynamic dynamic 5000 5000 search 2 remove 200 add 200 remove_and_add 200 >> raport3a.txt
	./main_dynamic static 5000 5000 search 2 remove 200 add 200 remove_and_add 200 >> raport3a.txt
	echo "Shared libraries: " >> raport3a.txt
	echo "______________________________" >> raport3a.txt
	./main_shared dynamic 5000 5000 search 2 remove 200 add 200 remove_and_add 200 >> raport3a.txt
	./main_shared static 5000 5000 search 2 remove 200 add 200 remove_and_add 200 >> raport3a.txt

clean:
	rm -f *.o *.a

clean_exe:
	rm -f *.exe *.so