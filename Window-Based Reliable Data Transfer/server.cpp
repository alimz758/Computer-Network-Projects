#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include<time.h>
#define BUFFER_SIZE 50000


//Main
int main(int argc, char *argv[]){
    //initializing to variables ti read the clients message
    struct stat stat;
    long read_value;
    int file_descriptor;
    int message_size=1024;
    char client_buffer_message[message_size];
    char * file_name=NULL;
    
    if (argc != 2) {
        fprintf(stderr, "ERROR! Please provide a port number\n");
        exit(1);
    }
    int port= atoi(argv[1]);;
    printf("\n\n\t\t\tSERVING ON PORT: %d\n\n", port);
    return 0;
}