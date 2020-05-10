# Simple Window-Based Reliable Data Transfer
Name: Ali Mirabzadeh
email: thealimz758@g.ucla.edu
UID: 305179067


## High Level Design

### UDP High Level Design :
UDP Server :
    
    Create UDP socket.
    Bind the socket to server address.
    Wait until datagram packet arrives from client.
    Process the datagram packet and send a reply to client.
    Go back to Step 3.

UDP Client :

    Create UDP socket.
    Send message to server.
    Wait until response from server is recieved.
    Process reply and go back to step 2, if necessary.
    Close socket descriptor and exit.

## Problems I Ran Into

## Resources Used

For UDP creation:
https://www.geeksforgeeks.org/udp-server-client-implementation-c/

To better understand 3-way handshake:
https://www.quora.com/Whats-the-difference-with-UDP-implementing-the-three-way-handshake-and-TCP

To get the hostname:
http://man7.org/linux/man-pages/man3/gethostbyname.3.html