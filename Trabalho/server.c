#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <unistd.h>  //close 

#include <netinet/in.h>

#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros  

#define PORT 1300

int main(){
    char server_message[256] = "You have reached the server.";

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


    int client_socket = accept(server_socket, NULL,NULL);    // At this point, connection is established between client and server, and they are ready to transfer data. 

    if (client_socket < 0) { 
        printf("server acccept failed...\n"); 
        exit(0); 
    } 
    else
        printf("server acccept the client...\n"); 

    send(client_socket, server_message, sizeof(server_message), 0);

    close(server_socket);  
    return 0;
}