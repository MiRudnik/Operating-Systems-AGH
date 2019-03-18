CC = gcc
CFLAGS = -Wall -std=gnu11

all: main_static main_dynamic main_shared test clean

static:
	$(CC) -c blockLibrary.c -o blockLibrary.o $(CFLAGS) $(OPT)
	ar rcs libBlockLibrary.a blockLibrary.o

shared:
	$(CC) -fPIC -c blockLibrary.c -o blockLibrary.o $(CFLAGS) $(OPT)
	$(CC) -fPIC -shared -o libBlockLibrary.so blockLibrary.o $(CFLAGS) $(OPT)

main_static:
	make main_static_O0
	make main_static_O1
	make main_static_O2

main_dynamic:
	make main_dynamic_O0
	make main_dynamic_O1
	make main_dynamic_O2

main_shared:
	make main_shared_O0
	make main_shared_O1
	make main_shared_O2

# -O0
main_static_O0:
	make static OPT=-O0
	$(CC) -o main_static_O0 main.c libBlockLibrary.a


main_dynamic_O0:
	make shared OPT=-O0
	$(CC) -o main_dynamic_O0 main_dyn.c -ldl


main_shared_O0:
	make shared OPT=-O0
	$(CC) -o main_shared_O0 main.c libBlockLibrary.so

#-O1
main_static_O1:
	make static OPT=-O1
	$(CC) -o main_static_O1 main.c libBlockLibrary.a


main_dynamic_O1:
	make shared OPT=-O1
	$(CC) -o main_dynamic_O1 main_dyn.c -ldl


main_shared_O1:
	make shared OPT=-O1
	$(CC) -o main_shared_O1 main.c libBlockLibrary.so

#-O2
main_static_O2:
	make static OPT=-O2
	$(CC) -o main_static_O2 main.c libBlockLibrary.a


main_dynamic_O2:
	make shared OPT=-O2
	$(CC) -o main_dynamic_O2 main_dyn.c -ldl


main_shared_O2:
	make shared OPT=-O2
	$(CC) -o main_shared_O2 main.c libBlockLibrary.so

test:
	echo "" > raport3b.txt
	echo "Static libraries: " >> raport3b.txt
	echo "______________________________" >> raport3b.txt
	echo "O0 : " >> raport3b.txt
	./main_static_O0 dynamic 5000 5000 search 2 remove 200 add 200 remove_and_add 200 >> raport3b.txt
	echo "O1 : " >> raport3b.txt
	./main_static_O1 dynamic 5000 5000 search 2 remove 200 add 200 remove_and_add 200 >> raport3b.txt
	echo "O2 : " >> raport3b.txt
	./main_static_O2 dynamic 5000 5000 search 2 remove 200 add 200 remove_and_add 200 >> raport3b.txt
	echo "Dynamic libraries: " >> raport3b.txt
	echo "______________________________" >> raport3b.txt
	echo "O0 : " >> raport3b.txt
	./main_dynamic_O0 dynamic 5000 5000 search 2 remove 200 add 200 remove_and_add 200 >> raport3b.txt
	echo "O1 : " >> raport3b.txt
	./main_dynamic_O1 dynamic 5000 5000 search 2 remove 200 add 200 remove_and_add 200 >> raport3b.txt
	echo "O2 : " >> raport3b.txt
	./main_dynamic_O2 dynamic 5000 5000 search 2 remove 200 add 200 remove_and_add 200 >> raport3b.txt
	echo "Shared libraries: " >> raport3b.txt
	echo "______________________________" >> raport3b.txt
	echo "O0 : " >> raport3b.txt
	./main_shared_O0 dynamic 5000 5000 search 2 remove 200 add 200 remove_and_add 200 >> raport3b.txt
	echo "O1 : " >> raport3b.txt
	./main_shared_O1 dynamic 5000 5000 search 2 remove 200 add 200 remove_and_add 200 >> raport3b.txt
	echo "O2 : " >> raport3b.txt
	./main_shared_O2 dynamic 5000 5000 search 2 remove 200 add 200 remove_and_add 200 >> raport3b.txt

clean:
	rm -f *.o *.a

clean_exe:
	rm -f *.so *.exe