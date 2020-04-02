// Programming with TCP/IP sockets
// There are a few steps involved in using sockets:
// Create the socket
// Identify the socket
// On the server, wait for an incoming connection
// Send and receive messages
// Close the socket
// #include <netinet/in.h>

// struct sockaddr_in {
//     short            sin_family;   // e.g. AF_INET
//     unsigned short   sin_port;     // e.g. htons(3490)
//     struct in_addr   sin_addr;     // see struct in_addr, below
//     char             sin_zero[8];  // zero this if you want to
// };

#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
const int PORT= 8000;
//content types
char * content_type[5]={
    "text/html",
    "text/plain",
    "image/jpg",
    "image/png",
    "application/octet-stream" //for binary if not specified
};
//define the HTTP response status code
char* http_status_code[5]={
    "200 OK",
    "301 Moved Permanently",
    "400 Bad Request",
    "404 Not Found",
    "505 HTTP Version Not Supported"
};
//to be used to create a socket
int create_socket_fd, new_socket; //int server_fd = socket(domain, type, protocol);
//domain: communication domain in which the socket should be created our case AF_INET (IP)
//type: type of service. This is selected according to the properties required by the application: SOCK_STREAM (virtual circuit service
//protocol: indicate a specific protocol to use in supporting the sockets operation. This is useful in cases 
//where some families may have more than one protocol to support a given type of service. The return value is a file descriptor (a small integer). 
char * response_content_type=NULL;
//helper function to check the file format and return the content type accordingly 
char * content_type_checker(char *file_name_copy){
    char* extention = NULL;
    int file_name_length = strlen(file_name_copy);
    int dot_index = -1;
    //find the dot position
    for (int i = file_name_length - 1; i >= 0; i--) {
        if (file_name_copy[i] == '.') {
            dot_index = i;
            break;
        }
    }
    //if no '.' found=> there is no extention
    if(dot_index==-1){
        return content_type[4];
    }
    //extract the extention
    else {
        int file_extension_length = file_name_length - dot_index;
        extention = malloc(sizeof(char) * file_extension_length);
        memset(extention, 0, sizeof(char) * file_extension_length);
        strncpy(extention, dot_index + file_name_copy + 1, file_extension_length - 1);
    }
    //return the content type corresponding to the extentions
    if(strcmp(extention, "html") == 0){
        return content_type[0];
    }
    else if(strcmp(extention, "txt") == 0){
        return content_type[1];
    }
    else if(strcmp(extention, "jpg") == 0){
        return content_type[2];
    }
    //png
    else{
        return content_type[3];
    }
}
//helper function to parse the client's request
char * request_parser(char *client_request){
    //if the message is null just igonre and return
    if(client_request==NULL){
        return NULL;
    }
    //otherwise get the first line of the message which contains the GET request part of the filename
    char * first_line_of_request = NULL;
    char * client_copy_request= NULL;
    char* file_name=NULL;
    char * first_line_disected;
    client_copy_request= client_request;
    first_line_of_request = strtok(client_copy_request,"\n");
    //disecting the first line to get the file name
    first_line_disected = strtok(first_line_of_request," ");
    while(first_line_disected){
        //check if there is a file name
        if(first_line_disected[0]=='/'){
            file_name= strtok(first_line_disected," ");
            break;
        }
        first_line_disected = strtok(NULL," ");
    }
    //remove '/' from the beginning
    if (file_name[0] == '/') {
        memmove(file_name, file_name+1, strlen(file_name));
    }
    //check if no file is specified 
    if(file_name[0]=='\0'){
        //set the file name to "landingPage to send index.html as the response"
        file_name="LandingPage";
    }
    //get the content type
    response_content_type= content_type_checker(file_name);
    return file_name;
}
int main(int argc, char const *argv[]){
    //initializing to variables ti read the clients message
    long read_value;
    int message_size=30000;
    char client_buffer_message[message_size];
    char * file_name=NULL;
    printf("\n\n\t\t\tSERVING ON PORT: %d\n\n", PORT);
    int backlog=5;
    //struct to be used for biding
    struct sockaddr_in socket_address;
    socklen_t address_len;
    //open to listen 
    create_socket_fd =socket(AF_INET, SOCK_STREAM, 0);
    if(create_socket_fd<0){
        printf("ERROR! Could not open a socket\n");
    }
    //fill the details
    memset((char *)&socket_address, 0, sizeof(socket_address)); 
    //The address family we used when we set up the socket. In our case, it’s AF_INET.
    socket_address.sin_family= AF_INET;
    //The port number (the transport address)
    socket_address.sin_port=htons(PORT);/* htons converts a short integer (e.g. port) to a network representation */ 
    //The address for this socket
    socket_address.sin_addr.s_addr = htonl(INADDR_ANY);/* htonl converts a long integer (e.g. address) to a network representation */ 
    //bind the socket
    int binding_socket = bind(create_socket_fd, (struct sockaddr *) &socket_address, sizeof(socket_address));
    if(binding_socket<0){
        printf( "ERROR! Could not bind the socket\n");
    }
    //The listen system call tells a socket that it should be capable of accepting incoming connections
    //The second parameter, backlog, defines the maximum number of pending connections that can be queued up before connections are refused.
    if (listen(create_socket_fd, backlog) < 0) { 
        printf("ERROR! Could not listen to the cocket\n");
        exit(EXIT_FAILURE); 
    }
    while(1){
        printf("\n\t\t+++++++ Waiting For New Connection ++++++++\n\n");
        if ((new_socket = accept(create_socket_fd, (struct sockaddr *)&socket_address, (socklen_t*)&address_len))<0){
            printf("ERROR! Accept failed\n");
            exit(EXIT_FAILURE);
        }
        //read the client's message with read() system call  ssize_t read(int fd, void *buf, size_t count);
        read_value = read(new_socket,client_buffer_message,message_size-1 );
        //if nothing to read
        if(read_value<0){
            close(create_socket_fd);
            close(new_socket);
            printf("ERROR! Could not read the client message\n");
        }
        else{
            printf(" Client's Message is\n %s\n", client_buffer_message);
            //START PROCESSING THE REQUEST
            //FIRST
            //GET THE FILE NAME FROM THE PATH
            file_name= request_parser(client_buffer_message);
            printf("File Name: %s\n" , file_name);
            printf("File Content Type: %s\n" , response_content_type);
            //TODO: OPEN FILE

            //TODO: SEND THE RESPONSE BACK THE CLIENT
        }
    }
    close(create_socket_fd);
    return 0;
}