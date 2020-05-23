#include "gbn.h"
//helper function to create a packet for client/server
int packet_generator(packet_info *packet, uint16_t seq_num, uint16_t ack_num, uint16_t payload_size,const void *data, bool flags[3], int pack_num ){
    //first check whether the packet_data_lenght is noth freater than 512 bytes
    if(payload_size > MAX_PAYLOAD_SIZE){
        fprintf(stderr, "ERROR! The payload size is greater than 512B\n");
        return -1;
    }
    //clear the packet from the previous call 
    memset(&packet->packet_header_pointer,0, sizeof(packet_header));
    memset(packet->data, 0, payload_size);
    //start structring the packet
    packet->packet_header_pointer.sequence_num= seq_num;
    packet->packet_header_pointer.ack_num=ack_num;
    packet->packet_header_pointer.ack_flag=flags[0]; //ACK
    packet->packet_header_pointer.fin_flag=flags[1]; // FIN
    packet->packet_header_pointer.syn_flag=flags[2];// SYN
    packet->packet_header_pointer.pack_num= pack_num;
    packet->packet_header_pointer.len=payload_size;
    //copy the data
    if(data != NULL && payload_size>0){
        memcpy(packet->data,data,payload_size );
    }
    return 0;
}
//helper function to clear the packet
void clear_packet(packet_info *packet){
    memset(packet, 0, sizeof(*packet));
}
//Helper function to generate a random number for ACK Number
int random_num_generator(){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    //using nano-seconds instead of seconds as this function gets called within a second on both server and client side
    //before using this method i was getting the same "random" for both
    srand((time_t)ts.tv_nsec);
    return (rand()% MAX_SEQUENCE_NUM )+ 1;  
}