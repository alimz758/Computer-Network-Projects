// Server side logic
// If a SYN packet, reply with packet with SYN flag and ACK flag, set ACK number field and sequence number field
// If a data packet, write data field to file
// If a FIN packet, reply with packet with ACK flag. Then send a packet with FIN flag. After receive ACK from client, close the connection.

#include "gbn.h"
int sockfd, new_sockfd;
void sig_handler(int signo) {
  if (signo == SIGINT) {
    close(sockfd);
    close(new_sockfd);
    printf("Recieved SIGINT\n");
  }
}
state client_state;
state server_state;
//to be used on the server for saving the file as CONNECTION-ORDER.file
int connection_number=0;
//helper function for the server to send ACK 
int server_handshake(int sockfd, struct sockaddr *client, socklen_t *socklen){
    //struct for storing the client respons
    packet_header client_response;
    clear_packet(&client_response);
    while(true){
        if((recvfrom(sockfd, (char *)&client_response, sizeof(packet_header), 0, client, socklen))==-1){
            fprintf(stderr, "ERROR! The server failed to receive the client's message\n");
            continue;
        }
        //check that whether it's a SYN and client SEQ_NUM +1 is indeed the expexted number
        else if(client_response.syn_flag ==true ){
            packet_info synack_packet;
            bool flags[3]={true, false, true};
            //initialize a random SEQ_number for the server 
            server_state.seq_num= random_num_generator();
            //store the Server's ACK num
            server_state.ack_num= client_response.sequence_num +1;
            //generating the SYNACK Packet
            if(packet_generator(&synack_packet,server_state.seq_num,server_state.ack_num,0, NULL, flags )){
                fprintf(stderr, "ERROR! Generating SYNACK failed\n");
            }
            //Sending the packet to the client
            if ((sendto(sockfd, &synack_packet, sizeof(synack_packet), 0, client, *socklen)) == -1) {
                fprintf(stderr, "ERROR! Server failed to send SYNACK \n");
            } else {
                printf("SEND %d %d SYN ACK\n", server_state.seq_num,server_state.ack_num);
                server_state.udp_state=SYN_ACK_SENT;
                //store the next expected ACK number that the server should expect from the client for the next packet
                server_state.next_expected_ack_num= server_state.seq_num+1;
                return sockfd;
            }
        }
    }
    return -1;
}
//Init the server state for listenting and ready to get packet nums
int server_listen(){
    memset(&server_state, 0, sizeof(server_state));
    server_state.udp_state=LISTENING;
    server_state.udp_role=SERVER;
    server_state.next_expected_ack_num=1;
    return 0;
}
//helper function to receive Data Packets from the server
int data_packet_recv(int sockfd, void *buf){
    //clear the buf from the  previous call
    memset(buf,0, MAX_PAYLOAD_SIZE);
    //retrieve the client's info
    struct sockaddr* client = server_state.client_ptr;
    socklen_t socklen = server_state.dest_socklen;
    return 0;
}
//------------------- SERVER ------------------
int main(int argc, char *argv[]){
    //initializing to variables ti read the clients message
    socklen_t socklen;
    int read_val;
    int output_file_saver_counter=1;
    FILE *output_file;
    char data_packet_buf[MAX_PAYLOAD_SIZE];
    if (argc <2 ) {
        fprintf(stderr, "ERROR! Please make sure to follow the following format: ./server <port>\n");
        exit(1);
    }
    int port= atoi(argv[1]);
    printf("\n\n\t\t\tSERVING ON PORT: %d\n\n", port);
    signal(SIGINT, sig_handler);
    //struct to be used for biding
    struct sockaddr_in server_socket, client_socket;
    //open to listen 
    sockfd =socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd<0){
        printf("ERROR! Could not open a socket\n");
        exit(EXIT_FAILURE); 
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
        exit(EXIT_FAILURE); 
    }
    //The listen system call tells a socket that it should be capable of accepting incoming connections
    //The second parameter, backlog, defines the maximum number of pending connections that can be queued up before connections are refused.
    if (server_listen() < 0) { 
        fprintf(stderr,"ERROR! Could not listen to the socket\n");
        exit(EXIT_FAILURE); 
    }
    //Waiting for the client to connect 
    socklen = sizeof(struct sockaddr_in);
    //waiting for the client to connect 
    if((new_sockfd =server_handshake(sockfd, (struct sockaddr *)&client_socket, &socklen))==-1){
        fprintf(stderr,"ERROR! Server could not establish the onnection with the client\n");
        exit(EXIT_FAILURE); 
    }
    //At this ponint connection is established so start receiving data
    //start  receiving data packets
    std::string file_name =  std::to_string(output_file_saver_counter) + ".file";
    const char * char_type_file_name = file_name.c_str();
    printf("file name: %s \n",file_name);
    //open file, <connection_num>.file to write
    if((output_file= fopen(char_type_file_name, "w"))==NULL){
        fprintf(stderr, "ERROR! Could not open file to write on the server side\n");
        exit(EXIT_FAILURE);
    }
    while(true){
        if((read_val=data_packet_recv(new_sockfd,data_packet_buf))==-1){
            fprintf(stderr,"ERROR! The server failed in receiving the Data Packets from the client\n");
            exit(EXIT_FAILURE);
        }
        else if(read_val==0){
            break;
        }
        //write form the data_buffer to the stream
        fwrite(data_packet_buf, 1, read_val, output_file);
    }


    //closing the output file
    if (fclose(output_file) == EOF){
        fprintf(stderr,"ERROR! The server could not close the output file stream\n");
        exit(EXIT_FAILURE);
    }
    return 0;
}