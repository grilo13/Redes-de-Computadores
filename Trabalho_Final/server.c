/** TCP SERVER **/
// server : socket() -> bind() -> listen() -> accept() -> read()/write() -> close()
// client :            socket()        -> connect()    -> read()/write() -> close()

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/select.h>

#define MAXLINE 512
#define MAX_CLIENT 1000
#define CHATDATA 512
#define NICKNAME 9
#define INVALID_SOCK -1

//char greeting[]="Bem-vindo à sala.\n";
char ERROR[]= "Erro.\n";
char quit[]="QUIT\n";
char list[]="LIST\n";
char message[]="MSSG";
char nickname[]="NICK";

// Client List 정보
struct List_c{
    int socket_num;         // Socket number
    char nick[NICKNAME];    // Client nickname 정보
    char ip[40];            // Client IP 정보
    int port;               // Server port 정보
}list_c[MAX_CLIENT];

// Armazenamento de índice de clientes conectados

// int pushClient(int connfd, char* c_nick, char* c_ip, int c_port){
int pushClient(int connfd, char* c_ip, int c_port){    

    int i;

    for(i=0;i<MAX_CLIENT;i++){
        if(list_c[i].socket_num==INVALID_SOCK){
            list_c[i].socket_num = connfd;
            //strcpy(list_c[i].nick,c_nick);
            strcpy(list_c[i].ip,c_ip);
            list_c[i].port=c_port;
            return i;
        }
    }

    if(i == MAX_CLIENT)
        return -1;
}

int popClient(int s)
{
    int i;


    for(i=0; i<MAX_CLIENT;i++){
        if(s==list_c[i].socket_num){
            list_c[i].socket_num=INVALID_SOCK;
            memset(list_c[i].nick,0,sizeof(list_c[i].nick));
            memset(list_c[i].ip,0,sizeof(list_c[i].ip));
            break;
        }
    }
    close(s);
    return 0;
}


int nick_func(char* chatData, int i){

    char* token=NULL;
    char buf1[MAXLINE];

    memset(buf1,0,sizeof(buf1));
    token=strtok(chatData, " ");
    printf("%s", token);
    char * end;
    return 0;

}


// connect
void constr_func(int i,int index){

    char buf1[MAXLINE];

    memset(buf1,0,sizeof(buf1));

    sprintf(buf1,"[%s is connected]\r\n",list_c[index].nick);
    write(list_c[i].socket_num,buf1,strlen(buf1));

    sprintf(buf1,"[%s is connected]\r\n",list_c[i].nick);
    write(list_c[index].socket_num,buf1,strlen(buf1));
}

// quit
void quit_func(int i){

    int j;
    char* token=NULL;
    char buf1[MAXLINE];

    memset(buf1,0,sizeof(buf1));

    printf("%s Bye Bye ~ %s\r\n",list_c[i].nick, list_c[i].ip);

    for(j=0; j<MAX_CLIENT;j++)
        if(j!=i && list_c[j].socket_num!=INVALID_SOCK){
            sprintf(buf1,"[%s Bye Bye ~]\r\n",list_c[i].nick);
            write(list_c[j].socket_num,buf1,strlen(buf1));
        }
}

// list
void list_func(int i){
    int j,cnt=0;
    char buf1[MAXLINE];

    memset(buf1,0,sizeof(buf1));
    for(j=0; j<MAX_CLIENT;j++)
        if(list_c[j].socket_num!=INVALID_SOCK)cnt++;
    sprintf(buf1,"[Visitantes atuais : %d]\r\n",cnt);
    write(list_c[i].socket_num,buf1,strlen(buf1));
    for(j=0; j<MAX_CLIENT;j++)
        if(list_c[j].socket_num!=INVALID_SOCK){
            sprintf(buf1,"[%s from %s: %d]\r\n",list_c[j].nick,list_c[j].ip,list_c[j].port);
            write(list_c[i].socket_num,buf1,strlen(buf1));
        }
}
// message
int message_func(char* chatData,int i){
    int j,message_sock;
    char* token=NULL;
    char buf1[MAXLINE];

    memset(buf1,0,sizeof(buf1));
    token=strtok(chatData, " ");
    char * end;

    if(strcmp(token, message)){
        if((end=strtok(NULL,"\n"))==NULL)
            sprintf(buf1,"%s",token);
        else sprintf(buf1,"%s %s",token,end);
        sprintf(chatData,"[%s] %s\r\n",list_c[i].nick,buf1);
        return 1;
    }
    else{
        token=strtok(NULL," ");
        for(j=0;j<MAX_CLIENT;j++)
            if(!strcmp(list_c[j].nick, token))
                message_sock=list_c[j].socket_num;
        token=strtok(NULL, "\n");
        memset(buf1, 0, sizeof(buf1));
        sprintf(buf1, "[message from %s] %s\r\n", list_c[i].nick, token);
        write(message_sock, buf1, strlen(buf1));
        return 0;
    }
}

void main(int argc, char *argv[])
{
    int newSockfd,sockfd;                      // Socket File Descriptor
    struct sockaddr_in servaddr;
    int maxfd=0;                            // Número de arquivos gerenciados por multiplexação de E / S
    int i,j,n;
    fd_set rset;

    int index;

    char* token=NULL;
    char buf1[MAXLINE];
    char buf2[MAXLINE];
    char chatData[CHATDATA];

    // ./server [port_number] Envia mensagem se nenhum argumento for fornecido
    if(argc<2){
        printf("usage: %s port_number\n",argv[0]);
        exit(-1);
    }

    // Socket - produção
    // SOCK_STREAM : TCP/IP protocolo
    if ((sockfd=socket(AF_INET,SOCK_STREAM,0)) <= 0){
        perror("socket");
        exit(1);
    }

    // Informações necessárias para o socket
    memset(&servaddr,0,sizeof(servaddr));       // Inicializar com 0x00 tanto quanto o tamanho do servaddr
    servaddr.sin_addr.s_addr=INADDR_ANY; // INADDR_ANY: Digite seu próprio endereço IP. Significa mapear qualquer endereço IP
    servaddr.sin_family=AF_INET;                // Protocolo de Internet IPv4
    servaddr.sin_port=htons(atoi(argv[1]));     //Use htons porque deve ser alterado para bytes de rede

    // Registre as informações necessárias para o Socket no kernel
    if(bind(sockfd,(struct sockaddr *) &servaddr,sizeof(servaddr)) < 0){
        printf("Error : Kill process.\n");
        exit(1);
    }

    // Confirmação de solicitação de conexão do cliente. Formal...
    if(listen(sockfd, MAX_CLIENT) < 0){
        printf("listen");
        exit(1);
    }

    // inicialização de socket_num
    for(i=0;i<MAX_CLIENT;i++)
        list_c[i].socket_num=INVALID_SOCK;

    // Inicializar buf1
    memset(buf1,0,sizeof(buf1));

    // Converte o endereço IP binário em um endereço IP decimal e armazena em buf1
    inet_ntop(AF_INET, &servaddr.sin_addr, buf1, sizeof(buf1));

    int save_port = (int) ntohs(servaddr.sin_port);

    printf("[Server address %s : %d] \n", buf1, ntohs(servaddr.sin_port));

    char *message = "Session started! \n set your nickname with command NICK first!";


    for ( ; ; )
    {
        maxfd=sockfd;

        FD_ZERO(&rset);              // Limpa o conjunto de select.
        FD_SET(sockfd, &rset);       //Coloca o sockfd no conjunto.
        for(i=0; i<MAX_CLIENT; i++){
            if(list_c[i].socket_num!=INVALID_SOCK){
                FD_SET(list_c[i].socket_num,&rset);
                if(list_c[i].socket_num > maxfd) maxfd=list_c[i].socket_num;
            }
        }
        maxfd++;

        if(select(maxfd, &rset, (fd_set *)0, (fd_set *)0, (struct timeval *)0) < 0){
            printf("Select error\n");
            exit(1);
        }

        // FD_ISSET: Em que estilo surgiu o Socket

        int addrlen = sizeof(servaddr);

        if(FD_ISSET(sockfd, &rset)){
            
            
            // Aguardar até que a conexão com o cliente seja estabelecida.
            // Quando aceite, um novo socket denominado newSockfd é criado. Envio e recebimento de dados usando newSockfd.
            if((newSockfd=accept(sockfd, (struct sockaddr *)&servaddr , (socklen_t *) &addrlen)) > 0) {

                //memset(buf1, 0, sizeof(buf1));
                memset(buf2, 0, sizeof(buf2));
               

                //read(newSockfd, buf1, sizeof(buf1));   // Ler o nick do cliente.

                inet_ntop(AF_INET, &servaddr.sin_addr, buf2, sizeof(buf2));


                //index = pushClient(newSockfd, buf1 , buf2, save_port);
                index = pushClient(newSockfd, buf2, save_port);

                message = "Session started! \n set your nickname with command NICK ! \n";

                if( send(newSockfd, message, strlen(message), 0) != strlen(message) ) {
                    
                    perror("send failed");
                }
              
                puts("...welcome message sent to NEW CLIENT");

          /*
                // Insirir sock_num, nick, ip, port_num do cliente
                printf("%s has been connected from %s\n", list_c[index].nick, list_c[index].ip );*/
                

                if(index < 0){
                    write(newSockfd,ERROR,strlen(ERROR));
                    close(newSockfd);
                }else{

                    //write(newSockfd, greeting, strlen(greeting));
                    for(i=0; i<MAX_CLIENT;i++) //avisa quem está conectado no canal ativo
                        if(i!=index && list_c[i].socket_num!=INVALID_SOCK){
                            constr_func(i, index);     
                        }
                }
            }
        }

        for(i=0; i<MAX_CLIENT;i++){
            if((list_c[i].socket_num != INVALID_SOCK) && FD_ISSET(list_c[i].socket_num,&rset)){
                memset(chatData, 0, sizeof(chatData));
                if((n=read(list_c[i].socket_num,chatData, sizeof (chatData)))>0){

                
                        // Enviar mensagem de bate-papo padrão
                        for(j=0; j<MAX_CLIENT;j++){   // send chatting letters
                            if(list_c[i].socket_num !=INVALID_SOCK)
                                if(j!=i)
                                    write(list_c[j].socket_num, chatData, sizeof(chatData));
                        }
                    
                        if(strstr(chatData, nickname) != NULL){
                            if(nick_func(chatData, i) == 0) continue;
                            else;
                        }
                    
                        // Encerrando a conexão [/ quit]
                        if(!strcmp(chatData, quit)){   // disconnect from the client "i"
                            quit_func(i);
                            popClient(list_c[i].socket_num);
                            continue;
                        }
                        // Client Mostrar lista [/ list]
                        if(!strcmp(chatData,list)){ //print the list of clients
                            list_func(i);
                            continue;
                        }
                        // Enviar mensagem 1:1 [/ mensagem [Cliente] [mensagem]]
                        if(strstr(chatData, message) != NULL){
                            if(message_func(chatData, i) == 0) continue;
                            else;
                        }



                } 
                  

                   /*

                    // Enviar mensagem de bate-papo padrão
                    for(j=0; j<MAX_CLIENT;j++){   // send chatting letters
                        if(list_c[i].socket_num !=INVALID_SOCK)
                            if(j!=i)
                                write(list_c[j].socket_num, chatData, sizeof(chatData));
                    }

                    // Encerrando a conexão [/ quit]
                    if(!strcmp(chatData, quit)){   // disconnect from the client "i"
                        quit_func(i);
                        popClient(list_c[i].socket_num);
                        continue;
                    }
                    // Client Mostrar lista [/ list]
                    if(!strcmp(chatData,list)){ //print the list of clients
                        list_func(i);
                        continue;
                    }
                    // Enviar mensagem 1:1 [/ mensagem [Cliente] [mensagem]]
                    if(strstr(chatData, message) != NULL){
                        if(message_func(chatData, i) == 0) continue;
                        else;
                    }
                    */

            }
        }
    }

}
