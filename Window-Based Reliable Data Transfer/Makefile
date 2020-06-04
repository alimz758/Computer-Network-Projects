
CC = g++
CFLAGS = -g -Wpointer-arith -Wsign-compare -Wall -Wextra -std=c++11 

default: 
	$(CC) $(CFLAGS) timer.cpp gbn.cpp server.cpp -o server
	$(CC) $(CFLAGS) timer.cpp gbn.cpp client.cpp -o client
dist:
	tar -z -cf 305179067.tar.gz timer.cpp sserver.cpp client.cpp gbn.h gbn.cpp Makefile README.md

clean:
	rm -f *.file *.o *.so *.tar.gz server client 