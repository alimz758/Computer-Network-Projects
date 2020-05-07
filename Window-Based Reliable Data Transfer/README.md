# Simple Window-Based Reliable Data Transfer



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
## Resources I used

For UDP creation:
https://www.geeksforgeeks.org/udp-server-client-implementation-c/