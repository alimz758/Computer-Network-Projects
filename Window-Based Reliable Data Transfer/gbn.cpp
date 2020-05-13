#include "gbn.h"
//helper function to create a packet for client/server
int packet_generator(packet_info *packet, int seq_num, int ack_num, int payload_size,const void *data, bool flags[3] ){
    //first check whether the packet_data_lenght is noth freater than 512 bytes
    if(payload_size > MAX_PAYLOAD_SIZE){
        return -1;
    }
    //clear the packet from the previous call before
    memset(packet, 0, sizeof(*packet));
    //start structring the packet
    packet->packet_header_pointer->sequence_num= seq_num;
    packet->packet_header_pointer->ack_num=ack_num;
    packet->packet_header_pointer->ack_flag=flags[0]; //ACK
    packet->packet_header_pointer->fin_flag=flags[1]; // FIN
    packet->packet_header_pointer->syn_flag=flags[2];// SYN
    //copy the data
    if(data != NULL && payload_size>0){
        memcpy(packet->data,data,payload_size );
    }
    return 0;
}
