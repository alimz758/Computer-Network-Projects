Name: Ali Mirabzadeh,
UID: 305179067,
Email: thealimz758@g.ucla.edu


HIGH LEVEL DESIGN:


Problems Faced:
I had issues with using strtok() function as I tried to be smart and parse the string with that but at some point it modified, the 

pointer points the modified string,  the orginal string that I would pass in so I could not access the original one later. So in 

content_type_checker() where I needed the original, I gave up on using strtok(), instead I used 'for loop' to find the location of the 

'.' to extract the extention. C/C++ pointers are annoying!


Resources Used:
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