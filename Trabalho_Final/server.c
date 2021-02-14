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

#define MAX_CLIENT 1000
#define MAXLINE 512
#define CHATDATA 512
#define NICKNAME 10
#define INVALID_SOCK -1
#define ROLESIZE 10

char ERROR[]= "Erro.\n";
char quit[]="QUIT\n";
char whos[]="WHOS\n";
char message[]="MSSG";
char nickname[]="NICK";
char kick[]="KICK\n";
char role[]="ROLE";
char user_info[]="INFO\n";
char message2[]="MAIN";
char exists[]="EXISTS\n";

// Client List
struct List_c{
    int socket_num;
    int first_command;         // Socket number
    char nick[NICKNAME];    // Client nickname 
    char ip[40];            // Client IP 
    int port;               // Server port
    char role[ROLESIZE];            
}list_c[MAX_CLIENT];

// Armazenamento de índice de clientes conectados

int pushClient(int connfd, char* c_ip, int c_port){    

    int i;
    char* user ="User";

    for(i=0;i<MAX_CLIENT;i++){
        if(list_c[i].socket_num==INVALID_SOCK){
            list_c[i].socket_num = connfd;
            strcpy(list_c[i].ip, c_ip);
            list_c[i].port = c_port;
            strcpy(list_c[i].role, user);
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


void set_nick(char* c_nick, int i){

    strcpy(list_c[i].nick,c_nick);
 
}

//NICK 
void nick_func(char* chatData, int i){

    char* token;
    char old_nick[NICKNAME];
    char buf1[MAXLINE];
    char* end = " ";
    int aux = 0;
    int aux2 = 0;
    int tamanho;


    memset(buf1,0,sizeof(buf1));

    
    token = strtok(chatData, end);

    if(strcmp(token, nickname)==0){
      token = strtok(NULL, end);
      token[strcspn(token, "\n")] = 0;
        if(strlen(token) == 0){
            if(list_c[i].socket_num!=INVALID_SOCK){
                puts("RPLY 002 - Erro: Falta introdução do nome.\n");
                sprintf(buf1,"RPLY 002 - Erro: Falta introdução do nome.\n");
                write(list_c[i].socket_num,buf1,strlen(buf1));
            }
        }else{
            if(list_c[i].socket_num!=INVALID_SOCK){
                if(strlen(token) > NICKNAME){
                    puts("RPLY 003 - Erro: Nome pedido não válido. (Excede nº máximo de carateres permitido ou utiliza carateres inválidos\n");
                    sprintf(buf1,"RPLY 003 - Erro: Nome pedido não válido. (Excede nº máximo de carateres permitido ou utiliza carateres inválidos\n");
                    write(list_c[i].socket_num,buf1,strlen(buf1));
                }else if(strlen(token) < NICKNAME){
                    for(int j = 0; j < MAX_CLIENT; j++){
                        if(strcmp(list_c[j].nick,token)==0){
                            puts("RPLY 004 - Erro: nome já em uso (num outro utilizador registado ou em uso por um utilizador não registado, e o comando não tem qualquer efeito\n");
                            sprintf(buf1,"RPLY 004 - Erro: nome já em uso (num outro utilizador registado ou em uso por um utilizador não registado, e o comando não tem qualquer efeito\n");
                            write(list_c[i].socket_num,buf1,strlen(buf1));
                            aux = aux + 1;
                            aux2 = 3;
                            break;
                        }
                    }
                    if(aux == 0){
                        tamanho = strlen(list_c[i].nick);
        
                        if(tamanho > 0){
                            strcpy(old_nick,list_c[i].nick);
                            set_nick(token, i);
                        
                        }else{
                            set_nick(token, i);
                            puts("RPLY 001 - Nome atribuído com sucesso\n");
                            sprintf(buf1,"RPLY 001 - Nome atribuído com sucesso\n");
                            write(list_c[i].socket_num,buf1,strlen(buf1));
                            printf("%s has been connected from %s\n", list_c[i].nick, list_c[i].ip);
                            aux2 = 1;
                        }
 
                    }
                }
            }
        }

    }else{
        if(list_c[i].socket_num!=INVALID_SOCK){
            puts("RPLY 002 - Erro: Falta introdução do nome.\n"); 
            sprintf(buf1,"RPLY 002 - Erro: Falta introdução do nome.\n");
            write(list_c[i].socket_num,buf1,strlen(buf1));
        }
    }

    if(aux2 == 1){
        for(int j = 0; j < MAX_CLIENT; j++){  
            if(list_c[j].socket_num!=INVALID_SOCK){
                if(j!=i){
                    sprintf(buf1,"MSSG server :> novo utilizador <%s>\r\n\n", list_c[i].nick);
                    write(list_c[j].socket_num,buf1,strlen(buf1));
                }
            }
        }
    }else if(aux2 == 0){
       for(int j = 0; j < MAX_CLIENT; j++){  
            if(list_c[j].socket_num!=INVALID_SOCK){
                if(j!=i){
                    sprintf(buf1,"MSSG server :> <%s> mudou o seu nome para <%s>\r\n", old_nick, list_c[i].nick);
                    write(list_c[j].socket_num,buf1,strlen(buf1));
                }
            }
        }
    }
}


//show the info of the users
void show_client_info(int i){
    int j;
    char* token=NULL;
    char buf1[MAXLINE];

    memset(buf1,0,sizeof(buf1));

    for(j=0; j<MAX_CLIENT;j++)
        if(j==i && list_c[j].socket_num!=INVALID_SOCK){
            sprintf(buf1,"User details\n-> %s\n-> %s\n-> %s\r\n" ,list_c[i].nick, list_c[i].role, list_c[i].ip);
            write(list_c[j].socket_num,buf1,strlen(buf1));
        }
}

//see if exists the name, then return the index of the socket user
int see_exists(int i, char* name){
    int j;
    char buf1[MAXLINE];

    int id;

    memset(buf1,0,sizeof(buf1));

    for(j=0; j<MAX_CLIENT;j++){
        if(list_c[j].socket_num!=INVALID_SOCK){
            if(strcmp(list_c[j].nick,name) == 0){
                id = j;
                sprintf(buf1,"ID: %d e Nome utilizador: %s\n", list_c[j].socket_num,list_c[j].nick);
                write(list_c[i].socket_num,buf1,strlen(buf1));
            } else {
                id = -1;  //não encontra utilizador com o nome pedido
            }
        }
    }

    return id; //retorna o id do utilizador encontrado, se for igual a -1, é porque não encontrou esse utilizador.
}


// QUIT
void quit_func(int i){
    int j;
    char* token=NULL;
    char buf1[MAXLINE];

    memset(buf1,0,sizeof(buf1));
    printf("%s logged out from the ip %s\r\n",list_c[i].nick, list_c[i].ip);
    for(j=0; j<MAX_CLIENT;j++)
        if(j!=i && list_c[j].socket_num!=INVALID_SOCK){
            sprintf(buf1,"%s Deixou de ser operador.\r\n",list_c[i].nick);
            write(list_c[j].socket_num,buf1,strlen(buf1));
        }
}

//GIVE ROLE TO USER
void give_role(char* chatData, int i){
    int j;
    char* token;
    char buf1[MAXLINE];
    int contador = 0;
    char aux2[20];
    char* end = " ";

    token=strtok(chatData, end);

    char* aux = "Operador";


    memset(buf1,0,sizeof(buf1));
    printf("%s \n", list_c[i].role);
    printf("%s wants to add a role \r\n",list_c[i].nick);

    if(strcmp(list_c[i].role,aux) == 0){
        printf("%s pode dar role operador a outro utilizador.\n", list_c[i].nick);
    } else {
        printf("%s não tem permissões para tal.\n", list_c[i].nick);
    }


   if(strcmp(token, role)==0){
        token = strtok(NULL, end);
        token[strcspn(token, "\n")] = 0;
        strcpy(aux2,token);
   }

    printf("AUX2 -> %s ", aux2);


    int id_user = see_exists(i,aux2);

    if(id_user == -1){
        for(j=0; j<MAX_CLIENT;j++)
            if(j==i && list_c[j].socket_num!=INVALID_SOCK){
                sprintf(buf1,"RPLY 802 – Erro. Ação não autorizada, utilizador cliente não é um operador %s.\r\n",list_c[id_user].role);
                write(list_c[j].socket_num,buf1,strlen(buf1));
            }

    } else {
        printf("ID USER %d", id_user);

        strcpy(list_c[id_user].role, aux);

        printf("USER %s %s\n", list_c[id_user].nick, list_c[id_user].role);
        for(j=0; j<MAX_CLIENT;j++)
            if(j==id_user && list_c[j].socket_num!=INVALID_SOCK){
                sprintf(buf1,"RPLY 801 – Foi promovido a %s.\r\n",list_c[id_user].role);
                write(list_c[j].socket_num,buf1,strlen(buf1));
            } else if(j==i && list_c[j].socket_num!=INVALID_SOCK){
                sprintf(buf1,"Promoveste a %s o utilizador %s\r\n",list_c[id_user].role, list_c[id_user].nick);
                write(list_c[j].socket_num,buf1,strlen(buf1));
            }
    }
}

//KICK
void kick_user(int i){
    int j,cnt=0;
    char buf1[MAXLINE];

    char* aux = "OPERADOR";

    if(strcmp(list_c[i].role,aux) == 0){
        memset(buf1,0,sizeof(buf1));

        for(j=0; j<MAX_CLIENT;j++)
            if(list_c[j].socket_num!=INVALID_SOCK){
                cnt++;
            }
        sprintf(buf1,"%s, tem permissões para kickar alguém. \r\n",list_c[i].nick);
        write(list_c[i].socket_num,buf1,strlen(buf1));

    } else {
        sprintf(buf1,"%s, não tem permissões para kickar alguém.\r\n", list_c[i].nick);
        write(list_c[i].socket_num,buf1,strlen(buf1));
        printf("%s, não tem permissões para tal.\n", list_c[i].nick);
    }   
}
// WHOS

void whos_func(int i){
    int j,cnt=0;
    char buf1[MAXLINE];

    memset(buf1,0,sizeof(buf1));

    for(j=0; j<MAX_CLIENT;j++)
        if(list_c[j].socket_num!=INVALID_SOCK)
            cnt++;
    sprintf(buf1,"[Visitantes atuais do canal : %d]\r\n",cnt);
    write(list_c[i].socket_num,buf1,strlen(buf1));

    for(j=0; j<MAX_CLIENT;j++){
        if(list_c[j].socket_num!=INVALID_SOCK){
            sprintf(buf1,"[%s from %s: %d]\r\n",list_c[j].nick,list_c[j].ip,list_c[j].port);
            write(list_c[i].socket_num,buf1,strlen(buf1));
        }
    }   
}

// message
void message_func(char* chatData,int i){

    char* token;
    char token2[MAXLINE];
    char buf1[MAXLINE];
    char mensagem[MAXLINE];
    char* end = " ";
    int aux = 0;
/*
    memset(buf1,0,sizeof(buf1));
    
    
    token = strtok(chatData, end);


    if(strcmp(token, message)==0){
        token = strtok(NULL, end);
        token[strcspn(token, "\n")] = 0; //remover \n 
        if(strlen(token) == 0){
            if(list_c[i].socket_num!=INVALID_SOCK){
                puts("RPLY 102 - Erro. Não há texto na mensagem.\n");
                sprintf(buf1,"RPLY 102 - Erro. Não há texto na mensagem.\n");
                write(list_c[i].socket_num,buf1,strlen(buf1));
            }
        }else{
            if(list_c[i].socket_num!=INVALID_SOCK){
                if(strlen(token) > CHATDATA){
                    puts("RPLY 103 - Erro. Mensagem demasiado longa.\n");
                    sprintf(buf1,"RPLY 103 - Erro. Mensagem demasiado longa.\n");
                    write(list_c[i].socket_num,buf1,strlen(buf1));
                }else if(strlen(token) < CHATDATA){
                    puts("RPLY 101 - mensagem enviada com sucesso.\n");
                    sprintf(buf1,"RPLY 101 - mensagem enviada com sucesso.\n");
                    write(list_c[i].socket_num,buf1,strlen(buf1));
                    /*strcpy(token2, token);
                    for(int j = 0; j < MAX_CLIENT; j++){  
                        if(list_c[j].socket_num!=INVALID_SOCK){
                            if(j!=i){
                                if(strstr(token, token2)==0){
                                    continue;   
                                }else{
                                    while(strstr(token, token2)!=0){
                                        token = strtok(NULL, end);
                                    }
                                }
                                while(token != NULL){
                                    strcat(token2, end);
                                    strcat(token2, token);
                                    token = strtok(NULL, end);
                                }
                            sprintf(buf1,"\nMSSG < %s/%s > : > %s \n", list_c[i].nick, list_c[i].role, token2);
                            write(list_c[j].socket_num,buf1,strlen(buf1));
                            }
                        }
                    }
                }

            }
        }
    }else{ 
        if(list_c[i].socket_num!=INVALID_SOCK){
            puts("RPLY 102 - Erro. Não há texto na mensagem.\n"); 
            sprintf(buf1,"RPLY 102 - Erro. Não há texto na mensagem.\n");
            write(list_c[i].socket_num,buf1,strlen(buf1)); 
        }
    }
*/
}

void removeFirst(char* str, const char * toRemove)
{
    int i, j;
    int len, removeLen;
    int found = 0;

    len = strlen(str);
    removeLen = strlen(toRemove);

    for(i=0; i<len; i++)
    {
        found = 1;
        for(j=0; j<removeLen; j++)
        {
            if(str[i+j] != toRemove[j])
            {
                found = 0;
                break;
            }
        }

        /* If word has been found then remove it by shifting characters  */
        if(found == 1)
        {
            for(j=i; j<=len-removeLen; j++)
            {
                str[j] = str[j + removeLen];
            }

            // Terminate from loop so only first occurrence is removed
            break;
        }
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

    char buf1[MAXLINE];
    char buf2[MAXLINE];
    char chatData[CHATDATA];
    char new_chatData[CHATDATA];

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

    printf("[Server address %s : %d]\n\n", buf1, ntohs(servaddr.sin_port));

    char *message2 = "Session started! \n set your nickname with command NICK first! \n\n";


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

                inet_ntop(AF_INET, &servaddr.sin_addr, buf2, sizeof(buf2));
             
                index = pushClient(newSockfd, buf2, save_port); //criar um cliente

                message2 = "Welcome! \n Set your nickname with command NICK first! --> NICK <nickname>  \n\n";

                if( send(newSockfd, message2, strlen(message2), 0) != strlen(message2) ) {
                    
                    perror("send failed");
                }
              
                puts("...welcome message sent to NEW CLIENT\n");
                

                if(index < 0){
                    write(newSockfd,ERROR,strlen(ERROR));
                    close(newSockfd);
                }
            }
        }


        for(i=0; i<MAX_CLIENT;i++){
            if((list_c[i].socket_num != INVALID_SOCK) && FD_ISSET(list_c[i].socket_num,&rset)){
                memset(chatData, 0, sizeof(chatData));
                if((n=read(list_c[i].socket_num,chatData, sizeof (chatData)))>0){

        
                    if(strstr(chatData, nickname) != NULL){
                        nick_func(chatData, i);
                        list_c[i].first_command = 1; 
                        continue;
                    }

                    if(list_c[i].first_command != 1){
                        memset(buf2, 0, sizeof(buf2));
                        sprintf(buf2,"NICK COMMMAND SHOULD BE FIRST \n\n");
                        write(list_c[j].socket_num,buf2,strlen(buf2));
                        continue;

                    }else{

                         // Encerrando a conexão [/ quit]
                        if(!strcmp(chatData, quit)){   // disconnect from the client "i"
                            quit_func(i);
                            popClient(list_c[i].socket_num);
                            continue;
                        }

                        // Client Mostrar lista [/ list]
                        if(!strcmp(chatData,whos)){ //print the list of clients
                            whos_func(i);
                            continue;
                        }

                       // Enviar mensagem 1:1 [/ mensagem [Cliente] [mensagem]]
                        if(strstr(chatData, message) != NULL){
                            memset(buf1,0, sizeof(buf1));
                            //strcpy(new_chatData, chatData)
                            //message_func(new_chatData, i);
                            removeFirst(chatData, message);
                            for(j=0; j<MAX_CLIENT;j++){   // send chatting letters
                                if(list_c[j].socket_num !=INVALID_SOCK){
                                    if(list_c[j].socket_num != list_c[i].socket_num){
                                        sprintf(buf1,"\nMSSG < %s/%s > : >%s \n", list_c[i].nick, list_c[i].role, chatData);
                                        write(list_c[j].socket_num,buf1,strlen(buf1));
                                    }
                                }
                            }
                            continue;
                        }

                        // Gives the user is information
                        if(!strcmp(chatData,user_info)){
                            show_client_info(i);
                            continue;
                        }

                        if(strstr(chatData, role) != NULL){
                            give_role(chatData, i);
                            continue;
                        }

                    }
                }
            }
        }
    }
}
