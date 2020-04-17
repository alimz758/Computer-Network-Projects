CC = gcc
CFLAGS = -g -Wall -Wextra
default: server.c	
	$(CC) $(CFLAGS)  -o server server.c

dist:
	tar -z -cf 305179067.tar.gz server.c Makefile README.md 404.html index.html test.html image2.jpg image1.png
clean:
	rm -f *.o *.so *.tar.gz server