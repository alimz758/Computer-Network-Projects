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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include<time.h>
#define BUFFER_SIZE 4096
const int PORT= 8000;
//content types
char * content_type[5]={
    "text/html",
    "text/plain",
    "image/jpg",
    "image/png",
    "application/octet-stream" //for binary if not specified
};
//define the HTTP response status code- based on the book I only put these
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
void sig_handler(int signo) {
  if (signo == SIGINT) {
    close(create_socket_fd);
    close(new_socket);
    printf("Recieved SIGINT\n");
  }
}
char * response_content_type=NULL;
//helper function to check the file format and return the content type accordingly 
char * content_type_checker(char *file_name_copy){
    char* extention = NULL;
    int file_name_length = strlen(file_name_copy);
    int dot_index = -1;
    //find the dot position
    if(strcmp(file_name_copy, "LandingPage") == 0){
        return content_type[0]; 
    }
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
//helper function to open the desired file
int open_desired_file(char * file_name){
    struct dirent* entry;
    int fd=-1;
    // char cwd[1024];
    // //get the current working directory
    // if (getcwd(cwd, sizeof(cwd)) == NULL) {
    //     printf("ERROR! getcwd() failed.\n");
    //     close(create_socket_fd);
    //     close(new_socket);
    //     return -1;
    // } 
    // printf("CWD is %s\n", cwd);
    // opendir() returns a pointer of DIR type.
    if(strcmp(file_name,"LandingPage")==0){
        fd= open("./index.html",O_RDONLY);
        if(fd<0){
            printf("Error! Could not load the Landing Page\n");
        }
        return fd;
    }  
    DIR *curr_directory = opendir("."); 
    if (curr_directory == NULL) { // opendir returns NULL if couldn't open directory 
        printf("Could not open current directory" ); 
        return -1; 
    } 
    // Refer http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html 
    // for readdir() 
    while ((entry = readdir(curr_directory)) != NULL) {
        //compare the file name with enry name, if the same then open with READ ONLY
        if (strcasecmp(file_name, entry->d_name) == 0) {
            printf("Current File Matched %s\n", entry->d_name);
            fd = open(entry->d_name, O_RDONLY);
            break;
        }
    }
    closedir(curr_directory);     
    return fd;
}
//helper fucntion to send the message to the client
void send_response_to_client(char *file_name, int fd, int socket,struct stat fd_stat){
    char * status=NULL;
    char http_response_message[BUFFER_SIZE];
    //if for any reason could not open a file, respond with 404
    if(fd<0){
        //404: NOT FOUND
        status= http_status_code[3];
        if ((fd = open("404.html", O_RDONLY)) < 0) {
            printf("ERROR! Could not open 404.html\n");

        }
        response_content_type= content_type[0];
    }
    else{
        //OK
        status= http_status_code[0];
    }
    // Get the date
    char s[1000];   
    time_t t = time(NULL);
    struct tm * p = localtime(&t);
    strftime(s, 1000, "%A, %B %d %Y", p);
    //start creating the response
    memset(http_response_message,0,BUFFER_SIZE);
    //if the file exist
    if(fd>0){
        //FOLLOW
        sprintf(http_response_message, 
            "HTTP/1.1 %s\r\nConnection: close\r\nDate: %s\r\nServer: C Web Server\r\nContent-Length: %lld\r\nContent-Type: %s\r\n\r\n",
            status, s, fd_stat.st_size,response_content_type);
    }
    //no open fd
    else{
        sprintf(http_response_message, 
            "HTTP/1.1 %s\r\nConnection: close\r\nDate: %s\r\nServer: C Web Server\r\nContent-Type: %s\r\n\r\n",
            status, s, response_content_type);
    }
    printf("Response message:\n %s\n", http_response_message);
    fflush(stdout);
    write(new_socket , http_response_message , strlen(http_response_message));
    // Send the filefile
    char file_buf[BUFFER_SIZE];
    memset(file_buf, 0, BUFFER_SIZE);
    long bytes_read;
    if (fd > 0) {
        while ((bytes_read = read(fd, file_buf, BUFFER_SIZE)) != 0) {
            if (bytes_read > 0) {
                if (write(new_socket, file_buf, bytes_read) < 0) {
                    printf("Error when writing to file\n");
                }
            }	
            else {
                printf("Error when reading file\n");
            }
        }
    }
    close(fd);
}
int main(){
    //initializing to variables ti read the clients message
    struct stat stat;
    long read_value;
    int file_descriptor;
    int message_size=30000;
    char client_buffer_message[message_size];
    char * file_name=NULL;
    printf("\n\n\t\t\tSERVING ON PORT: %d\n\n", PORT);
    int backlog=10;
    signal(SIGINT, sig_handler);
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
        printf("ERROR! Could not listen to the socket\n");
        exit(EXIT_FAILURE); 
    }
    while((new_socket = accept(create_socket_fd, (struct sockaddr *)&socket_address, (socklen_t*)&address_len))>0){
        printf("\n\t\t+++++++ Waiting For New Connection ++++++++\n\n");
        //read the client's message with read() system call  ssize_t read(int fd, void *buf, size_t count);
        read_value = read(new_socket,client_buffer_message,message_size-1 );
        //if nothing to read
        if(read_value<0){
            close(create_socket_fd);
            close(new_socket);
            printf("ERROR! Could not read the client message\n");
        }
        else{
            // printf(" Client's Message is\n %s\n", client_buffer_message);
            //START PROCESSING THE REQUEST
            //FIRST
            //GET THE FILE NAME FROM THE PATH
            file_name= request_parser(client_buffer_message);
            printf("Requested File Name: %s\n" , file_name);
            printf("File Content Type: %s\n" , response_content_type);
            //Open the desired file
            file_descriptor= open_desired_file(file_name);
            if(file_descriptor<0){
                printf("ERROR! Could not open the file!\n");
            }else{
                fstat(file_descriptor, &stat);
            }
            //TODO: SEND THE RESPONSE BACK THE CLIENT
            send_response_to_client(file_name,file_descriptor,new_socket,stat);
        }
        shutdown(new_socket,0);
    }
    shutdown(create_socket_fd,0);
    return 0;
}