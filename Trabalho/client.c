#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <unistd.h>

#include <netinet/in.h>

#define PORT 1300

int main(){

    //create a socket
    int network_socket;
    network_socket = socket(AF_INET, SOCK_STREAM, 0);

    //specify an address for the socket
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;   //connect to 00000


    int connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    //chek for error with the connection

    if(connection_status == -1){
        printf("THere was an error making a connection to the remote socket.");
    } else {
        printf("Connected to the remote socket. ");
    }

    //receive data from the server
    char server_response[256];
    recv(network_socket, &server_response, sizeof(server_response), 0);

    //print out the server's response
    printf("The server sent the data: %s\n", server_response);

    //and then close the socket
    close(network_socket);


    return 0;
}