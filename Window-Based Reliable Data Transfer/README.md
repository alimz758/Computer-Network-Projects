# Simple Window-Based Reliable Data Transfer
Name: Ali Mirabzadeh

email: thealimz758@g.ucla.edu

UID: 305179067


## High Level Design

### UDP High Level Design :
UDP Server :
    
    Create UDP socket.
    Bind the socket to server address.
    Wait for SYN from the client
    Send SYN_ACK
    Wait for Data-Packets
        Send data_ack for each received packet
        if didn't receive the expected packet
            send DUP-ACK
    Upon receiving FIN from client
        Save the file
        Send ACK to the client
        Send FIN to the client
    Close the Connection

UDP Client :

    Create UDP socket.
    Send SYN to the server
    Wait to receive SYN-ACK from the server
    Start Sending the data packets using GBN
        send packet within the window=10
        If didn't receive Data-ACK expected(or cumulative)
            TIMEOUT
                Resend packets from Base#-to-next_seq_num
        if received data-ACK
            change the expected accordingly
            changed the base number
    Send FIN after sending all data-packets
    Wait for the server's Fin
        Send ACK to the server
        Wait for 2-seconds
            redo the ACK if  received FIN from the server
    Close the connection

## Problems I Ran Into
1. rand(): I had issues using rand() as I was getting the same value on both the server and client, then I found a solution using CLOCK as 

the seed to srand().

2. data_packet  transmission: When I was doing 'diff' at the end, I would see the files are different and by checking the hexdump, I found out that

the server pads zeros at the end of the last packet as I was passing all packets with a default length of 512B. So I fixed that by sending the last packet 

exactly with its size and by creating a field called 'len' in the header to pass that to the server so the server only store the desired length

3. recvfrom() blocking: I had a problem on the client side where the program would hang and took me a while to realized it was the recvfrom() being

blocked and hangs if there is nothing to receive, so I made it non-blocking

4. Packet expected: In trasnmission of packets I had issues with keep ttracking of packets on the both side, so I created another field in the header

to keep track of the packets by integers from 0-to-NumberOfPacketsNeeded. That made it much easier so both client and the server just keeps

track of those number in their states and compare it with packet header 'pack_num' field.
## Resources Used

For UDP creation:

https://www.geeksforgeeks.org/udp-server-client-implementation-c/

To better understand 3-way handshake:

https://www.quora.com/Whats-the-difference-with-UDP-implementing-the-three-way-handshake-and-TCP

https://www.geeksforgeeks.org/tcp-3-way-handshake-process/

To get the hostname:

http://man7.org/linux/man-pages/man3/gethostbyname.3.html

For sending packets:

https://linux.die.net/man/2/sendto

For receiving the packet:

https://linux.die.net/man/2/recvfrom

For generating rand() with seed(nanosecond):

https://stackoverflow.com/questions/20201141/same-random-numbers-generated-every-time-in-c

For the timer:

https://gist.github.com/mcleary/b0bf4fa88830ff7c882d

fread():

http://www.cplusplus.com/reference/cstdio/fread/