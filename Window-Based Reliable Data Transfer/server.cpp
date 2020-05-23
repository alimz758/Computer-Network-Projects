// Server side logic
// If a SYN packet, reply with packet with SYN flag and ACK flag, set ACK number field and sequence number field
// If a data packet, write data field to file
// If a FIN packet, reply with packet with ACK flag. Then send a packet with FIN flag. After receive ACK from client, close the connection.

#include "gbn.h"
int sockfd, new_sockfd;
state client_state;
state server_state;
//to be used on the server for saving the file as CONNECTION-ORDER.file
int connection_number=0;
//helper function for the server to send ACK 
int server_handshake(int sockfd, struct sockaddr *client, socklen_t *socklen){
    //struct for storing the client respons
    packet_info client_response;
    server_state.client_ptr = client;
    server_state.dest_socklen = *socklen;
    clear_packet(&client_response);
    while(true){
        if((recvfrom(sockfd, (char *)&client_response, sizeof(packet_info), 0, client, socklen))==-1){
            fprintf(stderr, "ERROR! The server failed to receive the client's message\n");
            continue;
        }
        //check that whether it's a SYN and client SEQ_NUM +1 is indeed the expexted number
        else if(client_response.packet_header_pointer.syn_flag ==true ){
            fprintf(stdout, "RECV %d %d SYN\n", client_response.packet_header_pointer.sequence_num, client_response.packet_header_pointer.ack_num);
            packet_info synack_packet;
            bool flags[3]={true, false, true};
            //initialize a random SEQ_number for the server 
            server_state.seq_num =random_num_generator();
            //store the Server's ACK num
            server_state.ack_num= client_response.packet_header_pointer.sequence_num +1;
            //generating the SYNACK Packet
            if(packet_generator(&synack_packet,server_state.seq_num,server_state.ack_num,0, NULL, flags,0 )){
                fprintf(stderr, "ERROR! Generating SYNACK failed\n");
            }
            //Sending the packet to the client
            if ((sendto(sockfd, (char *)&synack_packet, sizeof(packet_info), 0, client, *socklen)) == -1) {
                fprintf(stderr, "ERROR! Server failed to send SYNACK \n");
            } else {
                printf("SEND %d %d SYN ACK\n", server_state.seq_num,server_state.ack_num);
                server_state.udp_state=SYN_ACK_SENT;
                //store the next expected ACK number that the server should expect from the client for the next packet
                server_state.next_expected_ack_num= server_state.ack_num;//the client's next seq_num is the expected number
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
    server_state.server_packet_expected=0;
    return 0;
}
//helper function to receive Data Packets from the server; a packet/time
int data_packet_recv(int sockfd, void *buf){
    //clear the buf from the  previous call
    memset(buf,0, MAX_PAYLOAD_SIZE);
    //retrieve the client's info
    struct sockaddr* client = server_state.client_ptr;
    socklen_t socklen = server_state.dest_socklen;
    //to store the packet that has been sent from the client
    packet_info client_data_packet;
    //generate the packet
    packet_info ack_packet;
    //TODO PUT THIS IN gbn.h
    bool ack_flag[3]={true, false, false};
    //start waiting for the client packet data
    while(true){
        //clear the client's packet buffer
        clear_packet(&client_data_packet);
        if((recvfrom(sockfd, (char *)&client_data_packet, sizeof(packet_info), 0, client, &socklen))==-1){
            fprintf(stderr, "ERROR! The server failed to receive the client's data packet\n");
        }
        //check whether it's an ACK Data packet AND it's SEQ_NUM is the expected data
        //this would be the first data_packet received
        else if( client_data_packet.packet_header_pointer.sequence_num== server_state.next_expected_ack_num){
            //copy the data_packet_payload into buffer
            server_state.udp_state= ACK_RCVD;
            int data_len= sizeof(&client_data_packet.data);
            memset(buf,0, data_len);
            memcpy(buf, &client_data_packet.data, data_len);
            //this would be for the first data_packet that would have an ACK
            if(client_data_packet.packet_header_pointer.ack_flag==true){
                fprintf(stdout, "RECV %d %d ACK\n", client_data_packet.packet_header_pointer.sequence_num, client_data_packet.packet_header_pointer.ack_num);
                //set up the Server SEQ NUMBER is it won't change ans stay the same after this point
                server_state.seq_num= client_data_packet.packet_header_pointer.ack_num;
            }
            //for the later data packet w/o ACK
            else{
                fprintf(stdout, "RECV %d 0\n", client_data_packet.packet_header_pointer.sequence_num);
            }
            //store the next expected SEQ_NUM from the server
            //do so by increasing it by data_payload_size
            
            server_state.next_expected_ack_num= server_state.next_expected_ack_num +data_len> MAX_SEQUENCE_NUM ? 0: server_state.next_expected_ack_num +data_len;
            //store the server ACK  number when sending the ACK packet to the client
            server_state.ack_num= client_data_packet.packet_header_pointer.sequence_num + data_len;

            packet_generator(&ack_packet, server_state.seq_num,server_state.ack_num,0,NULL,ack_flag,server_state.server_packet_expected);
            //send the ACK packet to the client
            if ((sendto(sockfd, &ack_packet, sizeof(ack_packet), 0, client, socklen)) == -1) {
                fprintf(stderr, "ERROR! Server failed to send DATA-ACK\n");
            } 
            else{
                fprintf(stdout, "SEND %d %d ACK\n", ack_packet.packet_header_pointer.sequence_num,ack_packet.packet_header_pointer.ack_num);
                server_state.server_packet_expected++;
                return data_len;
            }
        }
        //if the client sent FIN, indicating data packets are done
        else if(client_data_packet.packet_header_pointer.fin_flag==true){
            fprintf(stdout, "RECV %d %d FIN\n",client_data_packet.packet_header_pointer.sequence_num, client_data_packet.packet_header_pointer.ack_num);
            server_state.ack_num=client_data_packet.packet_header_pointer.sequence_num+1;
            server_state.next_expected_ack_num= server_state.ack_num;
            //generate ACK packet for the FIN received
            packet_generator(&ack_packet, server_state.seq_num,server_state.ack_num,0,NULL,ack_flag,0);
             //send the ACK packet to the client
            if ((sendto(sockfd, &ack_packet, sizeof(packet_info), 0, client, socklen)) == -1) {
                fprintf(stderr, "ERROR! Server failed to send ACK for the FIN received to the client \n");
            } 
            else{
                fprintf(stdout, "SEND %d %d ACK\n", ack_packet.packet_header_pointer.sequence_num,ack_packet.packet_header_pointer.ack_num);
                return 0;
            }
        } 
    }
    return -1;
}
int send_fin_packet(int sockfd){
    //retrieve the client's info
    struct sockaddr* client = server_state.client_ptr;
    socklen_t socklen = server_state.dest_socklen;
    packet_info client_data_packet;
    //Generate the FIN Packet
    packet_info fin_packet;
    bool fin_flags[3]={false,true,false};
    packet_generator(&fin_packet, server_state.seq_num, INIT_ACK_NUM,0,NULL, fin_flags,0);
    int min_attempt=1;
    int max_attempt=10;
    //try sending FIN for max 10 times
    while(min_attempt<=max_attempt){
        if ((sendto(sockfd, &fin_packet, sizeof(packet_info), 0, client, socklen)) == -1) {
            fprintf(stderr, "ERROR! Server failed to send FIN after sending its ACK\n");
        } 
        else{
            fprintf(stdout, "SEND %d 0 FIN\n", fin_packet.packet_header_pointer.sequence_num);
            if((recvfrom(sockfd, (char *)&client_data_packet, sizeof(packet_info), 0, client, &socklen))==-1){
                fprintf(stderr, "ERROR! The server failed to receive the client's data packet\n");
            }
            //check whether the received ACK is what the server was expecting for
            else if(client_data_packet.packet_header_pointer.ack_flag==true && client_data_packet.packet_header_pointer.sequence_num== server_state.next_expected_ack_num){
                //then close its connection
                fprintf(stdout, "RECV %d %d ACK\n", client_data_packet.packet_header_pointer.sequence_num, client_data_packet.packet_header_pointer.ack_num);
                server_state.udp_role=CLOSED;
                close(sockfd);
                return 0;
            }
        }
        min_attempt++;
    }
    return -1;
}
//------------------- SERVER ------------------
int main(int argc, char *argv[]){
    //initializing to variables ti read the clients message
    socklen_t socklen;
    int num_of_bytes_read;
    int output_file_saver_counter=1;
    FILE *output_file;
    char data_packet_buf[MAX_PAYLOAD_SIZE];
    if (argc <2 ) {
        fprintf(stderr, "ERROR! Please make sure to follow the following format: ./server <port>\n");
        exit(1);
    }
    int port= atoi(argv[1]);
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
    std::string file_name =  std::to_string(output_file_saver_counter)+".file";
    const char * char_type_file_name = file_name.c_str();
    //open file, <connection_num>.file to write
    if((output_file= fopen(char_type_file_name, "wb"))==NULL){
        fprintf(stderr, "ERROR! Could not open file to write on the server side\n");
        exit(EXIT_FAILURE);
    }
    while(true){
        if((num_of_bytes_read=data_packet_recv(new_sockfd,data_packet_buf))==-1){
            fprintf(stderr,"ERROR! The server failed in receiving the Data Packets/FIN from the client; didn't read any bytes\n");
            exit(EXIT_FAILURE);
        }
        else if(num_of_bytes_read==0){
            break;
        }
        //write form the data_buffer to the stream
        fwrite(data_packet_buf, 1, num_of_bytes_read, output_file);
    }
    //Closing the server side connection by sending FIN from the server
    if((send_fin_packet(sockfd))==-1){
        fprintf(stderr, "ERROR! The server failed sending its FIN to the clinet\n");
        exit(EXIT_FAILURE);
    }
    //closing the output file
    if (fclose(output_file) == EOF){
        fprintf(stderr,"ERROR! The server could not close the output file stream\n");
        exit(EXIT_FAILURE);
    }
    return 0;
}