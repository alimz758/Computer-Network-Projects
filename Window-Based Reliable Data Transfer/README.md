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

# How to Run

After cloning the repo, go to this directory, `cd Window-Based Reliable Data Transfer`, then run `make`.

`make` will generate two executables 'client' and 'server' and three binary files (100B, 1MB, 10MB) for testing.

You could generate your own binary file vy using `cat /dev/urandom | head -c <SIZE_OF_FILE> > <NAME_OF_FILE>`

Note that this implementation only handles files up to 10MB

Now you are ready to run the client and server, so open two terminals; one for the client and one for the server

Note that there is no parallelism in this implementation and the server processes clients sequentially and save

the files according to their connection order `<CONNECTION-ORDER>.file`. Check out the specs for more details

Server-Side:

    ./server <PORT_NUMBER> : ./server 5000

Client-Side:

    ./client <IP_ADDRESS> <PORT_NUMBER_USED_ON_THE_SERVER> <FILE> : ./client localhost 5000 one_MB

Note if you'd like to run the program with packet loss you need access to `tc` command in which you need a root access

on a Linux machine; I recommend Ubuntu on a virtual machine if your host is not Linux

Check out this website on how to use `tc`: http://www.linuxfoundation.org/collaborate/workgroups/networking/netem.

Finally, after client closes its connection you can check whether the server saved exactly the file as the original

For instnance if the client tried to send `one_MB` and this connection was the first connection that the server received,

you could run the following to test whether they are the same or not:

    `diff 1.file one_MB`

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


