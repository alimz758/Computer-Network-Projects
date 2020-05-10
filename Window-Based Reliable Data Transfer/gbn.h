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
//needs to be 12bytes
struct packet_header{
    int sequence_num; 
    int ack_num;
    bool ack_flag,
        syn_flag,
        fin_flag;
    bool nothing; //just for padding pusposes to be 12-bytes
};
char data[PAYLOAD_SIZE];

#endif