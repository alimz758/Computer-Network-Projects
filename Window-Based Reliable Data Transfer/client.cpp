// Client side logic
// Send a packet with SYN to initiate the connection.
// After receive packet with ACK, start send packets with data.
// After transmitting the entire file, send FIN packet and wait for ACK.
// After receive server FIN, send ACK and wait for 2 seconds to close the connection.
#include "gbn.h"
char *send_buffer_packet;
//------------------- CLIENT ------------------
//OVERVIEW OF THE CLIENT
// The client opens UDP socket, implements outgoing connection management, and connects to the server.
// Once connection is established, it sends the content of a file to the server.
#define MAX_NUMBER_OF_ATTEMPTS 10
state client_state;
state server_state;
//helper function to initiate the first handshake
//the client sends a SYN,  waits for SYNACK From the server, then the connection is established
//then the client send the payload
//if SYNACK was not received after a while, timeout, it will give up
int handshake_connection(int sockfd, struct sockaddr *server, socklen_t socklen){
    //Make the sockfd non-blocking
    fcntl(sockfd, F_SETFL, O_NONBLOCK);
    int sender_counter=0;
    //create a SYN packet
    packet_info syn_packet;
    bool flags[3]={false,false,true};
    //create struct to store the packet received from the server
    packet_info server_packet_response; //to store server response packet info
    clear_packet(&server_packet_response);
    //store the server info in client server pointer
    memcpy(&client_state.server, server, socklen);
    client_state.server_ptr = (struct sockaddr*) &client_state.server;
    client_state.dest_socklen = socklen;
    //try to connect to the server, first handshake
    int random_seq_num= random_num_generator();
    Timer time;
    //packet_generator(packet_info *packet, int seq_num, int ack_num, int payload_size,const void *data, bool flags[3] )
    packet_generator(&syn_packet,random_seq_num,INIT_ACK_NUM,0,NULL,flags,0);
    //keep trying for max 10 times to send SYN packet
    while(true){
        //send packet to the client
        if((sendto(sockfd,&syn_packet,sizeof(syn_packet), 0, server, socklen))!=-1){
            fprintf(stdout, "SEND %d %d SYN\n", random_seq_num, INIT_ACK_NUM);
            client_state.udp_state=SYN_SENT;
            //start the timer
            time.start();
            while(!time.isRTO()){
                //WAITING FOR SYNACK from the server
                if((recvfrom(sockfd, (char *)&server_packet_response, sizeof(packet_info), 0, server, &socklen))!=-1 
                    &&server_packet_response.packet_header_pointer.ack_flag==true && server_packet_response.packet_header_pointer.syn_flag==true ){
                    //format: RECV SeqNum AckNumi [SYN] [FIN] [ACK]
                    fprintf(stdout, "RECV %d %d SYN ACK\n", server_packet_response.packet_header_pointer.sequence_num, server_packet_response.packet_header_pointer.ack_num);
                    client_state.udp_state = ESTABLISHED;
                    //set the client's SEQ and ACK fields for the transmission datas
                    client_state.seq_num= server_packet_response.packet_header_pointer.ack_num;
                    client_state.ack_num=server_packet_response.packet_header_pointer.sequence_num+1;
                    time.reset();
                    return 0;
                }
            }
            //if TIMOUT and didn't RECV SYN ACK, resend the SYN packet
            if(server_packet_response.packet_header_pointer.ack_flag!=true &&server_packet_response.packet_header_pointer.syn_flag!=true ){
                fprintf(stdout, "TIMEOUT %d\n", random_seq_num);
                //stope timer
                time.reset();
                if((sendto(sockfd,&syn_packet,sizeof(syn_packet), 0, server, socklen))!=-1){
                    //start the timer again
                    time.start();
                    fprintf(stdout, "RESEND %d %d SYN\n", random_seq_num, INIT_ACK_NUM);
                    client_state.udp_state=SYN_SENT;
                }
            }
        }
        else{
            fprintf(stderr,"ERROR! Client could not send its SYN Packet for the %d time\n", sender_counter+1);
        }
    }
    fprintf(stderr, "The client faild to send SYN after 10 attempts.\n");
    return -1;
}
//helper function to receive the data packer from the server
int data_packet_recv(int sockfd){
    //TODO HANDLE DUP-ACK TO RESEND WITH APPROPRAITE
    //Turn off non-blocking mode (i.e. make recvfrom blocking again) */
    fcntl(sockfd, F_SETFL, O_NONBLOCK);
    //create a place to store server_packet header
    packet_info server_response;
    clear_packet(&server_response);
    //receive the data failed
    if((recvfrom(sockfd, (char *)&server_response, sizeof(packet_info), 0, client_state.client_ptr, &client_state.dest_socklen))==-1){
        //fprintf(stdout, "ERROR! RECV failed for data ACK!\n");
    }
    //if receive ack  for data packet and the ACK number is what we expected then
    else if(server_response.packet_header_pointer.ack_flag==true){
        fprintf(stdout,"RECV %d %d ACK\n", server_response.packet_header_pointer.sequence_num,server_response.packet_header_pointer.ack_num);
        //increase the client expected num and received packets
        if(client_state.client_packet_number_expected == server_response.packet_header_pointer.pack_num){
            client_state.next_expected_ack_num+=MAX_PAYLOAD_SIZE;
            client_state.client_packet_number_expected++;
            //printf("received packet #%d , base # %d\n", client_state.client_packet_number_expected,client_state.window_base_num);
            client_state.window_base_num=server_response.packet_header_pointer.pack_num+1;
            return 0;
        }
        //if got greater than what expected
        else if(client_state.client_packet_number_expected < server_response.packet_header_pointer.pack_num){
            client_state.next_expected_ack_num+=MAX_PAYLOAD_SIZE;
            client_state.client_packet_number_expected=server_response.packet_header_pointer.pack_num+1;
            //printf("received packet #%d , base # %d\n", client_state.client_packet_number_expected,client_state.window_base_num);
            client_state.window_base_num=server_response.packet_header_pointer.pack_num+1;
            return 0;
        }
       // printf("received out of order/dup packet # %d\n", client_state.client_packet_number_expected);
        //for out of order
        return 1; 
    }
    return -1;
}
//helper fucntion to send data packets after establishing 
int send_data_packet(int sockfd, const char * send_buffer_packet,size_t len){
    //store how many bites will be stored for the last packet
    int last_payload_len = len==MAX_PAYLOAD_SIZE? len : len % MAX_PAYLOAD_SIZE;
    int number_of_packets_needed;
    if(last_payload_len==0){
        number_of_packets_needed = len / MAX_PAYLOAD_SIZE;
    }
    else{
        number_of_packets_needed = (len / MAX_PAYLOAD_SIZE)+1;
    }
    //creating each packets data buffer
    int counter;
    packet_info data_packet;
    int packet_offset= 0;
    bool ack_flags[3]={true,false,false}; //Only ACK Flag
    bool no_flags[3]={false,false,false};
    //clear the packet buffer tracker of the client
    memset(&client_state.packet_buffer_tracker[0],0, sizeof(client_state.packet_buffer_tracker[0]));
    //send the FIRST DATA_PACKET with SEQ_NUM AS THE SAME AS THE ACK_NUM it received
    packet_generator(&data_packet,client_state.seq_num , client_state.ack_num, number_of_packets_needed==1? last_payload_len: MAX_PAYLOAD_SIZE, send_buffer_packet+ packet_offset,ack_flags,0 );
    memcpy(&client_state.packet_buffer_tracker[0], &data_packet, sizeof(data_packet));
    packet_offset+= MAX_PAYLOAD_SIZE;//in case there are more than 1 and going inside the following loop
    //set the client expexted number with should receive ACK with the SEQ it sent
    client_state.next_expected_ack_num = client_state.seq_num + sizeof(data_packet.data);
    //genereate each data packet header and data_payload
    for(counter=1; counter< number_of_packets_needed; counter++){
        //if the last packet
        if(counter +1 == number_of_packets_needed && last_payload_len!=0){
            client_state.seq_num=  sequence_number_calculator( client_state.seq_num,last_payload_len);
            packet_generator(&data_packet,client_state.seq_num , INIT_ACK_NUM,last_payload_len, send_buffer_packet+ packet_offset,no_flags,counter );
        }
        //not the last packet
        else{
            client_state.seq_num= sequence_number_calculator( client_state.seq_num,MAX_PAYLOAD_SIZE);
            packet_generator(&data_packet,client_state.seq_num , INIT_ACK_NUM ,MAX_PAYLOAD_SIZE, send_buffer_packet+ packet_offset,no_flags,counter );
            //increase the offset by 512B for the next packet
            packet_offset+= MAX_PAYLOAD_SIZE;
        }
        memset(&client_state.packet_buffer_tracker[counter],0, sizeof(client_state.packet_buffer_tracker[counter]));
        //set each client_packet's data
        memcpy(&client_state.packet_buffer_tracker[counter], &data_packet, sizeof(data_packet));
    }
    client_state.window_base_num=0;
    client_state.next_seq_num=0;
    //retrieve the server info from previously store client_state
    //be used when sending DATA Packet to the server
    struct sockaddr* server = client_state.server_ptr;
    socklen_t socklen = client_state.dest_socklen;
    Timer timer;
    int retransmite_counter;

    // -------- SENDING THE PACKETS ---------------
    //loop starting from the base_num until reaching to number of packets that are there to be sent
    while(client_state.window_base_num < number_of_packets_needed){
        //loop through the current window and send the packet
        while( client_state.next_seq_num< number_of_packets_needed &&  client_state.next_seq_num < SWS + client_state.window_base_num){
            //make sure the base number doesn't exceed the existing number of packets
            //send the packet
            if((sendto(sockfd, &client_state.packet_buffer_tracker[client_state.next_seq_num ],sizeof(packet_info), 0, server, socklen))==-1){
                fprintf(stderr,"ERROR! Could not send DATA_Packet with base_num: %d, SEQ_NUM: %d\n",client_state.next_seq_num, client_state.seq_num );
            }
            //for the first data packet with ACK Field
            else if(client_state.packet_buffer_tracker[client_state.next_seq_num ].packet_header_pointer.ack_flag==true){
                fprintf(stdout,"SEND %d %d ACK\n", 
                    client_state.packet_buffer_tracker[client_state.next_seq_num ].packet_header_pointer.sequence_num,client_state.packet_buffer_tracker[client_state.next_seq_num].packet_header_pointer.ack_num );
            }
            else{
                fprintf(stdout,"SEND %d 0\n", client_state.packet_buffer_tracker[client_state.next_seq_num ].packet_header_pointer.sequence_num);
            }
            if(client_state.next_seq_num == client_state.window_base_num)
                timer.start();
            client_state.next_seq_num++;
        }
        if(timer.isRTO()){
            timer.reset();
            printf("TIMEOUT %d\n",client_state.packet_buffer_tracker[client_state.window_base_num ].packet_header_pointer.sequence_num );
            //retransmite [base, next_seq_num-1]
            for(retransmite_counter = client_state.window_base_num; retransmite_counter<client_state.next_seq_num; retransmite_counter++){
                if((sendto(sockfd, &client_state.packet_buffer_tracker[retransmite_counter ],sizeof(packet_info), 0, server, socklen))==-1){
                    fprintf(stderr,"ERROR! Could not send RESEND DATA_Packet with base_num: %d, SEQ_NUM: %d\n", retransmite_counter, client_state.packet_buffer_tracker[retransmite_counter ].packet_header_pointer.sequence_num);
                }
                //for the first data packet with ACK Field
                else if(client_state.packet_buffer_tracker[retransmite_counter ].packet_header_pointer.ack_flag==true){
                    fprintf(stdout,"RESEND %d %d ACK\n", 
                        client_state.packet_buffer_tracker[retransmite_counter ].packet_header_pointer.sequence_num,client_state.packet_buffer_tracker[retransmite_counter].packet_header_pointer.ack_num );
                }
                else{
                    fprintf(stdout,"RESEND %d 0\n", client_state.packet_buffer_tracker[retransmite_counter].packet_header_pointer.sequence_num);
                }
            }
            timer.start();
        }
        if(data_packet_recv(sockfd)==0){
            timer.start();
        }
        //return when reached the end and received all the packets
        if( (number_of_packets_needed==1 ||client_state.window_base_num+1==number_of_packets_needed )){
            return 0;
        }   
    }
    return -1;
}
//helper function to close the connection on the  Clinet' side
int client_send_fin_packet(int sockfd){
    packet_info fin_packet;
    packet_info server_response;
    clear_packet(&server_response);
    bool flags[3]={false, true, false};
    //create the clietn FIN packet in case need for resending
    packet_generator(&fin_packet,client_state.seq_num , INIT_ACK_NUM,0,NULL,flags,0);
    //retrieve the  server info
    struct sockaddr* server = client_state.server_ptr;
    socklen_t socklen = client_state.dest_socklen;
    Timer fin_timer;
    //try sending the FIN packet
    //int try_counter=1;
    while(true){
        if ((sendto(sockfd, &fin_packet, sizeof(packet_info), 0, server, socklen)) == -1){
            fprintf(stderr, "ERROR! The client could not send its FIN Packet\n");
        }
        else {
            printf("SEND %d 0 FIN\n", client_state.seq_num);
            client_state.udp_state = FIN_SENT;
            fin_timer.start();
            //set the client seq number for the next packet sending
            if(client_state.seq_num +1 > MAX_SEQUENCE_NUM){
                client_state.seq_num=0;
            }
            else{
                client_state.seq_num+=1;
            }
            break;
        }
    }
    //waiting to receive first ACK , then FIN from the server
    while(true){
        if((recvfrom(sockfd, &server_response, sizeof(packet_header), 0, server, &socklen))==-1){
            //fprintf(stderr, "ERROR! The client failed in receiving the ACK after sending its FIN\n");
        }
        else if( fin_timer.elapsedSeconds()< TIMEOUT &&  server_response.packet_header_pointer.ack_flag==true){
            fin_timer.reset();
            printf("RECV %d %d ACK\n", server_response.packet_header_pointer.sequence_num, server_response.packet_header_pointer.ack_num);
            client_state.udp_state= ACK_RCVD;
            break;
        }
        //RESEND FIN if TIMEOUT or didn't receive ACK
        else{
            if ((sendto(sockfd, &fin_packet, sizeof(packet_info), 0, server, socklen)) == -1){
                    fprintf(stderr, "ERROR! The client could not send its FIN Packet\n");
            }
            else {
                //start the fin_timer
                //stop in fin function
                fin_timer.start();
                printf("RESEND %d 0 FIN\n", client_state.seq_num);
                client_state.udp_state = FIN_SENT;
                break;
            }
        }
    }
    //genereate the Client's ACK after receivinf FIN from the server
    packet_info client_ack_closing;
    bool flags1[3]={true, false,false};
    //generating the ACK packet
    packet_generator(&client_ack_closing,client_state.seq_num, client_state.ack_num,0, NULL,flags1,0);
    Timer time;
    while(true){
        clear_packet(&server_response);
        //wait for FIN from the server
        if((recvfrom(sockfd, &server_response, sizeof(packet_header), 0, server, &socklen))==-1){
            fprintf(stderr, "ERROR! The client failed in receiving the FIN after receiving ACK from the server\n");
        }
        else if(server_response.packet_header_pointer.fin_flag==true){
            printf("RECV %d %d FIN\n", server_response.packet_header_pointer.sequence_num, server_response.packet_header_pointer.ack_num);
            client_state.udp_state= FIN_RCVD;
            //stert the timer
            //wait for FIN from the server
            client_state.ack_num= server_response.packet_header_pointer.sequence_num +1;
            time.start();
            //set a timer
            if ((sendto(sockfd, &client_ack_closing, sizeof(packet_info), 0, server, socklen)) == -1){
                fprintf(stderr, "ERROR! The client could not send its ACK  Packet for closing\n");
            }
            else{
                fprintf(stdout,"SEND %d %d ACK\n", client_state.seq_num, client_state.ack_num);
                client_state.udp_state =ACK_SENT;
            }
            while(time.elapsedSeconds()<2.0){
                //non-blocking sockets. In non-blocking mode, recvfrom will return an error when there is no data, instead of waiting. 
                fcntl(sockfd, F_SETFL, O_NONBLOCK);
                clear_packet(&server_response);
                //wait for FIN from the server
                if((recvfrom(sockfd, &server_response, sizeof(packet_header), 0, server, &socklen))==-1){
                    //fprintf(stderr, "ERROR! The client failed in receiving the FIN after receiving ACK from the server\n");
                }
                else if(server_response.packet_header_pointer.fin_flag==true){
                    printf("RECV %d %d FIN\n", server_response.packet_header_pointer.sequence_num, server_response.packet_header_pointer.ack_num);
                    client_state.udp_state= FIN_RCVD;
                    if ((sendto(sockfd, &client_ack_closing, sizeof(packet_info), 0, server, socklen)) == -1){
                        fprintf(stderr, "ERROR! The client could not send its ACK  Packet for closing\n");
                    }
                    else{
                        fprintf(stdout,"SEND %d %d ACK\n", client_state.seq_num, client_state.ack_num);
                        client_state.udp_state =ACK_SENT;
                    }
                }
            }
            time.reset();
            client_state.udp_state=CLOSED;
            return close(sockfd);
        }
    }
    return -1;
}
int main(int argc, char *argv[]){
    int sockfd; 
    socklen_t socklen;
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
    if((file= fopen(argv[3],"rb"))==NULL){
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
    //------------------------  HANDSHAKE --------------------
    if(handshake_connection(sockfd,(struct sockaddr *)&servaddr, socklen)==-1){
        fprintf(stderr, "ERROR! Could not init the first handshake\n");
        exit(-1);
    }    
    //Pipelining and sending the file
    //size_t fread ( void * ptr, size_t size, size_t count, FILE * stream );
    //Read block of data from stream
    long len;
    // obtain file size:
    long lSize;
    fseek (file , 0 , SEEK_END);
    lSize = ftell (file);
    rewind (file);
    //allocate memory for the buffer
    send_buffer_packet = (char*) malloc (sizeof(char)*lSize);
    while((len=fread(send_buffer_packet,1,lSize, file))>0){
        if((send_data_packet(sockfd, send_buffer_packet,len)==-1)){
            fprintf(stderr, "ERROR! Failed to send the data packet\n");
            exit(-1);
        }
    }
    //closing connection
    //After sending all the packets the client would Send FIN
    //wait for ACK then FIN from the server
    //terminated after 2
    if((client_send_fin_packet(sockfd))==-1){
        fprintf(stderr, "ERROR! The clietn failed in closing the connection!\n");
        exit(EXIT_FAILURE);
    }
   
    //close the  file
    if (fclose(file) == EOF){
		fprintf(stderr, "ERROR! Failed to close the file\n");
		exit(EXIT_FAILURE);
	}
    free(send_buffer_packet);
    return 0;
}