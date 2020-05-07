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
#define MAXLINE 1024
//OVERVIEW OF THE CLIENT
// The client opens UDP socket, implements outgoing connection management, and connects to the server.
// Once connection is established, it sends the content of a file to the server.

//this is struct to hold all the flags needed
struct packet{
    //packet type numbers are as follows:
    //Datagram: 1
    //ACK: 2
    //Retransmission: 3
    int packet_type;
    int sequence_number;
    int ack_number;

};

int main(int argc, char *argv[]){
    int sockfd; 
    char buffer[MAXLINE]; 
    struct sockaddr_in  servaddr; 
    if (argc != 4) {
        fprintf(stderr, "ERROR! Please follow the following format: ./client <HOSTNAME-OR-IP> <PORT> <FILENAME> \n");
        exit(1);
    }
    int port= atoi(argv[2]);;
   
    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
    //seting up the server info
    memset(&servaddr, 0, sizeof(servaddr)); 
      
    // Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(port); 
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");//localhost 
      
      
    
  
    close(sockfd);
    return 0;
}