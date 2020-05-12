#include "gbn.h"
//------------------- SERVER ------------------
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
    printf("\n\n\t\t\tThe Server is Serving on Port: %d\n\n", port);
    return 0;
}