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
#include<time.h>
#define PAYLOAD_SIZE 512
#define MAX_SEQUENCE_NUM 25600
#define MAX_RTO 500 //in msec
#define WINDOW_SIZE 5
#define BUFFER_SIZE 1024
// ROLES
#define CLIENT 1
#define SERVER 2
//needs to be 12bytes
struct packet_header{
    int sequence_num; 
    int ack_num;
    bool ack_flag,
        syn_flag,
        fin_flag;
    bool nothing; //just for padding pusposes to be 12-bytes
}packet_header;
//packet info 524 bytes including the payload
struct packet_info{
    struct packet_header;
    char data[PAYLOAD_SIZE];
};
//keep track of state
struct state{
    int udp_state;
    int udp_role;
    struct sockaddr_storage client;
    struct sockaddr *client_ptr;
    struct sockaddr_storage server;
    struct sockaddr *server_ptr;
    socklen_t dest_socklen;
    int next_expected_pack_num;  /* Server/Receiver: packet number after the highest in sequence packet  */
    int window_start;            /* Client/Sender: the highest packet number that server has not yet DATAACKED */
    int recv_ack_timeout_count;
    int window_size;
}state;
//diferent modes
enum {
	CLOSED=0,
	LISTENING,
	SYN_SENT,
	SYN_RCVD,
	ESTABLISHED,
	FIN_SENT,
	FIN_RCVD
};
#endif