#include "gbn.h"
int sockfd, new_socket;
void sig_handler(int signo) {
  if (signo == SIGINT) {
    close(sockfd);
    close(new_socket);
    printf("Recieved SIGINT\n");
  }
}
//------------------- SERVER ------------------
int main(int argc, char *argv[]){
    //initializing to variables ti read the clients message
    struct stat stat;
    long read_value;
    int file_descriptor;
    int message_size=1024;
    char client_buffer_message[message_size];
    char * file_name=NULL;
    socklen_t socklen;

    if (argc !=3 ) {
        fprintf(stderr, "ERROR! Please make sure to follow the following format: ./client <port> <filename>\n \n");
        exit(1);
    }
    int port= atoi(argv[1]);;
    printf("\n\n\t\t\tSERVING ON PORT: %d\n\n", port);
    int backlog=1;
    signal(SIGINT, sig_handler);
    //struct to be used for biding
    struct sockaddr_in server_socket, client_socket;
    socklen_t address_len;
    //open to listen 
    sockfd =socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd<0){
        printf("ERROR! Could not open a socket\n");
    }
    //fill the details
    memset((char *)&server_socket, 0, sizeof(server_socket)); 
    //The address family we used when we set up the socket. In our case, it’s AF_INET.
    server_socket.sin_family= AF_INET;
    //The port number (the transport address)
    server_socket.sin_port=htons(port);/* htons converts a short integer (e.g. port) to a network representation */ 
    //The address for this socket
    server_socket.sin_addr.s_addr = htonl(INADDR_ANY);/* htonl converts a long integer (e.g. address) to a network representation */ 
    //bind the socket
    int binding_socket = bind(sockfd, (struct sockaddr *) &server_socket, sizeof(server_socket));
    if(binding_socket<0){
        fprintf( stderr,"ERROR! Could not bind the socket\n");
    }
    //The listen system call tells a socket that it should be capable of accepting incoming connections
    //The second parameter, backlog, defines the maximum number of pending connections that can be queued up before connections are refused.
    if (listen(sockfd, backlog) < 0) { 
        fprintf(stderr,"ERROR! Could not listen to the socket\n");
        exit(EXIT_FAILURE); 
    }
    //Waiting for the client to connect 
    socklen = sizeof(struct sockaddr_in);
    //TODO: RECVE SYN, SEND SYNACK
    if((new_socket = accept(sockfd, (struct sockaddr *)&client_socket, &socklen))==-1){
        fprintf(stderr, "ERROR! Could not bind the address\n");
        exit(EXIT_FAILURE); 
    }
    return 0;
}