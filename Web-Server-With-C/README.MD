Name: Ali Mirabzadeh,

UID: 305179067,

Email: thealimz758@g.ucla.edu


## HIGH LEVEL DESIGN:

1. Create the socket
2. Identify the socket
3. On the server, wait for an incoming connection
4. Receive request/message
5. Parse the request
6. Extract the needed information for header response (content_type, file name, etc)
7. Open a file descriptor
8. Send the HTTP response, Header
9. Close the socket

## Problems Faced:
In content_type_checker() I was using strtok() and strchr() to find the '.' and extract the extention but strtok() modifies the pointers 

location and as the result of that the original string gets modified. I was trying to be smart but didn't work and I could not fix it. 

Hence, I used a smiple 'for loop' to find the '.' and extract the extention without modifying the original string


## Resources Used:

To start and have a better undestanding of the project I read the following article

https://medium.com/from-the-scratch/http-server-what-do-you-need-to-know-to-build-a-simple-http-server-from-scratch-d1ef8945e4fa

For sockadd_in the library that I used for the internet address  

https://www.gta.ufrj.br/ensino/eel878/sockets/sockaddr_inman.html

For HTTP response header content

https://developer.mozilla.org/en-US/docs/Glossary/Response_header

For string parsing, used for parsing the HTTP request

https://c-for-dummies.com/blog/?p=1758
https://stackoverflow.com/questions/19555434/how-to-extract-a-substring-from-a-string-in-c
https://stackoverflow.com/questions/10651999/how-can-i-check-if-a-single-char-exists-in-a-c-string