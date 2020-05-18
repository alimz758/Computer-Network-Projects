
CC = g++
CFLAGS = -g -Wpointer-arith -Wsign-compare -Wall -Wextra -std=c++11 

default: 
	$(CC) $(CFLAGS) gbn.cpp server.cpp -o server
	$(CC) $(CFLAGS) gbn.cpp client.cpp -o client
dist:
	tar -z -cf 305179067.tar.gz server.cpp client.cpp gbn.h gbn.cpp Makefile README.md

clean:
	rm -f *.o *.so *.tar.gz server client 