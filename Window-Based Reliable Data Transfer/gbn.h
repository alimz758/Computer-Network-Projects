#ifndef _gbn_h
#define _gbn_h
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
#include <netdb.h>
#include <iostream>
#include<time.h>
#include <chrono>
#include "timer.cpp"
#define MAX_PAYLOAD_SIZE 512
#define MAX_SEQUENCE_NUM 25600 //as desired by the assignment
#define TIMEOUT 0.5 //0.5 sec
#define MAX_WINDOW_SIZE 5120  //10 packets per window
#define BUFFER_SIZE 1024
// ROLES
#define CLIENT 1
#define SERVER 2
//sender window size
#define SWS 10
//Client sends and ACK_NUM=0 for the first hand-shake
#define INIT_ACK_NUM 0
//needs to be 12bytes
typedef struct packet_header{
    uint16_t sequence_num; 
    uint16_t ack_num;
    int pack_num;
    bool ack_flag,
        syn_flag,
        fin_flag;
}packet_header;
//packet info 524 bytes including the payload
typedef struct packet_info{
    struct  packet_header packet_header_pointer;
    char *data;
}packet_info;
//keep track of state
typedef struct state {
    int udp_state;
    int udp_role;
    struct sockaddr_storage client;
    struct sockaddr *client_ptr;
    struct sockaddr_storage server;
    struct sockaddr *server_ptr;
    socklen_t dest_socklen;
    int next_expected_ack_num;  
    //the base number in GBN SWS
    int client_packet_number_expected;
    int server_packet_expected;
    int next_seq_num;
    int window_base_num ;   
    int recv_ack_timeout_count;
    uint16_t seq_num;
    uint16_t ack_num;
    int packet_num;
    packet_info packet_buffer_tracker[MAX_WINDOW_SIZE]; //TODO: NEED TO FIX THIS TO HOLD EXACTLY WHAT YOU NEED
}state;
//different modes
enum {
	CLOSED=0,
	LISTENING,
	SYN_SENT,
	SYN_RCVD,
	ESTABLISHED,
	FIN_SENT,
	FIN_RCVD,
    ACK_SENT,
    ACK_RCVD,
    SYN_ACK_SENT
};
int packet_generator(packet_info *packet, uint16_t seq_num, uint16_t ack_num, int payload_size,const void *data, bool flags[],int  );
void clear_packet(packet_info *);
int random_num_generator();
#endif