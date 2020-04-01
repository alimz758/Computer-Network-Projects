// Programming with TCP/IP sockets
// There are a few steps involved in using sockets:
// Create the socket
// Identify the socket
// On the server, wait for an incoming connection
// Send and receive messages
// Close the socket
// #include <netinet/in.h>

// struct sockaddr_in {
//     short            sin_family;   // e.g. AF_INET
//     unsigned short   sin_port;     // e.g. htons(3490)
//     struct in_addr   sin_addr;     // see struct in_addr, below
//     char             sin_zero[8];  // zero this if you want to
// };

#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
const int PORT = 8000;
//define the common file formats
char* file_format[4]={
    "html",
    "txt",
    "jpg",
    "png"
};
//define the HTTP response status code
char* http_status_code[5]={
    "200 OK",
    "301 Moved Permanently",
    "400 Bad Request",
    "404 Not Found",
    "505 HTTP Version Not Supported"
};
//to be used to create a socket
int create_socket_fd, new_socket; //int server_fd = socket(domain, type, protocol);
//domain: communication domain in which the socket should be created our case AF_INET (IP)
//type: type of service. This is selected according to the properties required by the application: SOCK_STREAM (virtual circuit service
//protocol: indicate a specific protocol to use in supporting the sockets operation. This is useful in cases 
//where some families may have more than one protocol to support a given type of service. The return value is a file descriptor (a small integer). 

int main(int argc, char const *argv[]){
    printf("Seriving on Port: %d", PORT);
    long valread;
    int backlog=5;
    //struct to be used for biding
    struct sockaddr_in socket_address;
    socklen_t address_len;
    //open to listen 
    create_socket_fd =socket(AF_INET, SOCK_STREAM, 0);
    if(create_socket_fd<0){
        fprintf(stderr, "ERROR! Could not open a socket");
    }
    //fill the details
    memset((char *)&socket_address, 0, sizeof(socket_address)); 
    //The address family we used when we set up the socket. In our case, it’s AF_INET.
    socket_address.sin_family= AF_INET;
    //The port number (the transport address)
    socket_address.sin_port=htons(PORT);/* htons converts a short integer (e.g. port) to a network representation */ 
    //The address for this socket
    socket_address.sin_addr.s_addr = htonl(INADDR_ANY);/* htonl converts a long integer (e.g. address) to a network representation */ 
    //bind the socket
    int binding_socket = bind(create_socket_fd, (struct sockaddr *) &socket_address, sizeof(socket_address));
    if(binding_socket<0){
        fprintf(stderr, "ERROR! Could not bind the socket");
    }
    //The listen system call tells a socket that it should be capable of accepting incoming connections
    //The second parameter, backlog, defines the maximum number of pending connections that can be queued up before connections are refused.
    if (listen(create_socket_fd, backlog) < 0) { 
        fprintf(stderr, "ERROR! Could not listen to the cocket");
        exit(EXIT_FAILURE); 
    }
    while(1){
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        if ((new_socket = accept(create_socket_fd, (struct sockaddr *)&socket_address, (socklen_t*)&address_len))<0){
            perror("In accept");
            exit(EXIT_FAILURE);
        }
        
        char buffer[30000] = {0};
        char *hello = "Hello from the server";
        valread = read( new_socket , buffer, 30000);
        printf("%s\n",buffer );
        write(new_socket , hello , strlen(hello));
        printf("------------------Hello message sent-------------------\n");
        close(new_socket);
    }
    return 0;
}