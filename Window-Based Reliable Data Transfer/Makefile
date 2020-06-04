
CC = g++
CFLAGS = -g -Wpointer-arith -Wsign-compare -Wall -Wextra -std=c++11 

default: 
	$(CC) $(CFLAGS) timer.cpp gbn.cpp server.cpp -o server
	$(CC) $(CFLAGS) timer.cpp gbn.cpp client.cpp -o client
	cat /dev/urandom | head -c 100 > hund_byte_binary
	cat /dev/urandom | head -c 1000000 > one_MB
	cat /dev/urandom | head -c 10000000 > ten_MB
dist:
	tar -z -cf 305179067.tar.gz timer.cpp server.cpp client.cpp gbn.h gbn.cpp Makefile README.md

clean:
	rm -f *.file *.o *.so *.tar.gz server client hund_byte_binary one_MB ten_MB 