#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <netinet/in.h>
#define PORT 8080

int main(){

    printf("--> Client Side <--  \n");

    struct hostent *host_info;
    struct in_addr *address;
    struct sockaddr_in serv_addr;

    char input[20] = "time.nist.gov";
    //printf("Will do a DNS query on: %s\n", input);

    host_info = gethostbyname(input);
    address = (struct in_addr *)(host_info->h_addr);

    bcopy( (char*)host_info->h_addr, 
            (char*)&serv_addr.sin_addr.s_addr,
            host_info->h_length);
            
    //printf("%s has address %s\n", input, inet_ntoa(*address));

    int sock = 0;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){ 
        printf("\n Socket creation error \n"); 
        return -1; 
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT); 
}



