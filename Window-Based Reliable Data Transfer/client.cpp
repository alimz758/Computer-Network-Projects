#include "gbn.h"
state client_state;
state server_state;
packet_info server_packet_response; //to store server response packet info
//------------------- CLIENT ------------------
//OVERVIEW OF THE CLIENT
// The client opens UDP socket, implements outgoing connection management, and connects to the server.
// Once connection is established, it sends the content of a file to the server.
#define MAX_NUMBER_OF_ATTEMPTS 10
//Helper function to generate a random number for ACK Number
int random_num_generator(){
    return rand()% MAX_SEQUENCE_NUM + 1; // 
}
//helper function to initiate the first handshake
//the client sends a SYN,  waits for SYNACK From the server, then the connection is established
//then the client send the payload
//if SYNACK was not received after a while, timeout, it will give up
int handshake_connection(int sockfd, const struct sockaddr *server, socklen_t socklen){
    fprintf(stdout, "Starting the first handshake\n");
    int sender_counter=0;
    //create a SYN packet
    packet_info syn_packet;
    bool flags[false,false,true];
    //create struct to store the packet received from the server
    
    //try to connect to the server, first handshake
    int random_seq_num= random_num_generator();
    if(packet_generator(&syn_packet,random_seq_num,INIT_ACK_NUM,0,NULL,flags)<0){
        fprintf(stderr,"ERROR! Client could not create its SYN Packet\n");
        return -1;
    }
    //keep trying for max 10 times to send SYN packet
    while(sender_counter< MAX_NUMBER_OF_ATTEMPTS){
        //send packet to the client
        if((sendto(sockfd,&syn_packet,sizeof(syn_packet), 0, server, socklen))!=-1){
            fprintf(stdout, "SEND %d %d SYN\n", random_seq_num, INIT_ACK_NUM);
            client_state.udp_state=SYN_SENT;
            return 0;
        }
        else{
            fprintf(stderr,"ERROR! Client could not send its SYN Packet for the %d time\n", sender_counter+1);
        }
        sender_counter++;
        //wait for a sec
        sleep(1);
    }
    return -1;
}
int main(int argc, char *argv[]){
    int sockfd; 
    socklen_t socklen;
    char buffer[BUFFER_SIZE]; 
    struct sockaddr_in  servaddr; 
    struct hostent *resolved_hostname;  /* structure for resolving names into IP addresses */
    socklen = sizeof(struct sockaddr);
    FILE *file;
    if (argc != 4) {
        fprintf(stderr, "ERROR! Please follow the following format: ./client <HOSTNAME-OR-IP> <PORT> <FILENAME> \n");
        exit(-1);
    }
    int port= atoi(argv[2]);
    //open the file, non-text file
    if((file= fopen(argv[3],"r"))==NULL){
        fprintf(stderr, "ERROR! Could not open the desired file!");
        exit(-1);
    }
    //Resolve the HOSTNAME 
    //The gethostbyname() function returns a structure of type hostent for the given host name.
    if((resolved_hostname=  gethostbyname(argv[1]))==NULL){
        fprintf(stderr, "ERROR! Could not resolve the hostname");
        exit(-1);
    }
    // Creating socket file descriptor using datagram
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        fprintf(stderr,"socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
    //seting up the server's params
    memset(&servaddr, 0, sizeof(servaddr)); 
    // Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(port); 
    servaddr.sin_addr =  *(struct in_addr *)resolved_hostname->h_addr;//host-name :what specified by the user as <HOSTNAME-OR-IP>
    //initiate the handshake
    //transmitting a SYN packet and waiting for an SYNACK packet
    //------------------------ SENDING THE FIRST HANDSHAKE --------------------
    if(handshake_connection(sockfd,(struct sockaddr *)&servaddr, socklen)==-1){
        fprintf(stderr, "ERROR! Could not init the first handshake\n");
        exit(-1);
    }
      
    
  
    close(sockfd);
    return 0;
}