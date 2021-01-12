#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#include <sys/types.h>
#include <sys/socket.h>

#include <unistd.h>  //close 

#include <netinet/in.h>

#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros  

#define PORT 1300
#define BUFSIZE 256

int main(int argc, char const *argv[]){
    char server_message[256] = "You have reached the server.";

    fd_set all_fds;   // master file descriptor list
    fd_set sel_fds;   // temp file descriptor list for select()
    int maxfd;        // maximum file descriptor number

    int opt = 1;      // for setsockopt() SO_REUSEADDR, below
    char buffer[BUFSIZE];
    int bytes;

    int new_socket; 
    //create the server socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully created..\n"); 

    //define the server address
    struct sockaddr_in server_address;
    int addrlen = sizeof(server_address); 
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    //bind the socket to our specified IP and port
    int b = bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));    //Bind the socket to server address. 

    if (b == -1) { 
        printf("Bind error...\n"); 
        exit(0); 
    } 
 
    //listen(server_socket, 5);    //put the server socket in a passive mode, where it waits for the client to approach the server to make a connection

    if ((listen(server_socket, 5)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    } 
    else
        printf("Server listening..\n"); 

    FD_ZERO(&all_fds);            // clear the master and temp sets
    FD_SET(server_socket, &all_fds);  // add socket to set of fds

    maxfd = server_socket;            // for now, the max is this one
  

    /*int client_socket = accept(server_socket, NULL,NULL);    // At this point, connection is established between client and server, and they are ready to transfer data. 

    if (client_socket < 0) { 
        printf("server acccept failed...\n"); 
        exit(0); 
    } 
    else
        printf("server acccept the client...\n"); 

    send(client_socket, server_message, sizeof(server_message), 0);

    close(server_socket);  */
    while(1) {  // Server loop
        int i;
        
        sel_fds = all_fds; // copy set of fds (because select() changes it)
        
        // Wait for data in ONE or MORE sockets
        if ( select(maxfd+1, &sel_fds, NULL, NULL, NULL) == -1 ) {
        perror("select failed");
        exit(EXIT_FAILURE);
        }

        // If we got here, there's data somewhere...
        // Let's find where

        for(i=0; i<=maxfd; i++) {
            if (FD_ISSET(i, &sel_fds)) { // we got one!!!
                // Now if it's the main socket, a client is connecting
                // If not, a client must have sent data (or disconnected)

                if (i == server_socket) {
                // A connection is ready to be accept()ed
                    if ((new_socket = accept(server_socket, (struct sockaddr *)&server_address,  
                                            (socklen_t*)&addrlen)<0)) { 
                        perror("accept failed"); 
                        exit(EXIT_FAILURE); 
                    }

            printf("Client connected.\n");

            // But now we have to monitor this socket too
            FD_SET(new_socket, &all_fds);

            // and update the maxfd, if necessary
            maxfd = new_socket > maxfd ? new_socket : maxfd;          
            }
            else {
            // here, we have data in one client, must recv() it
            bzero(buffer, BUFSIZE);
            
            bytes = recv(i, buffer, BUFSIZE-1, 0);

            if ( bytes == 0 ) { // client disconnected, too bad...
                FD_CLR(i, &all_fds);
                // nevermind the maxfd...
            }
            else {
                printf("Client says: '%s'\n", buffer);
                send(i, buffer, strlen(buffer), 0 );
                printf("Replied to client\n");

                if ( strcmp(buffer, "quit") == 0 ) {
                // client wants to quit, let it
                close(i);
                FD_CLR(i, &all_fds);              
                }      
            }
            }
        }
    }
  }
  
    return 0;
}