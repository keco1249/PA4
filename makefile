all: linkedlist.o
	gcc -Wall linkedlist.o client_PFS.c -o client_PFS
	gcc server_PFS.c -o server_PFS

linkedlist.o: linkedlist.h

clean:
	rm -f client_PFS server_PFS linkedlist.o
