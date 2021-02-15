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

#define MAX_CLIENT 10
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
char kick[]="KICK";
char role[]="OPER";
char user_info[]="INFO\n";
char message2[]="MAIN";
char exists[]="EXISTS\n";
char register1[]="REGS";
char list_regs[]="LISTR\n";
char authenticate[]="PASS";
char admin[]="ADMI";

// Client List
struct List_c{
    int socket_num;
    int first_command;       // Socket number
    char nick[NICKNAME];    // Client nickname 
    char ip[40];            // Client IP 
    int port;               // Server port
    char role[ROLESIZE];            
}list_c[MAX_CLIENT];

//Clients registered list
struct Register_c{    
    int socket_num;         // Socket number
    char nick[CHATDATA];
    char password[CHATDATA];
    char role[ROLESIZE];
    char auth[ROLESIZE];   //para saber se o cliente esta autenticado
}register_c[MAX_CLIENT];

// Armazenamento de índice de clientes conectados

//Funcao para dar operador a um ADMIN
void give_admin_role(char* chatData, int i){
    int j;
    char* token;
    char buf1[MAXLINE];
    int contador = 0;
    char aux2[20];
    char* end = " ";

    token=strtok(chatData, end);

    char* aux = "operador";
    char* aux4 ="autenticado";

    memset(buf1,0,sizeof(buf1));

   if(strcmp(token, role)==0){
        token = strtok(NULL, end);
        token[strcspn(token, "\n")] = 0;
        strcpy(aux2,token);
   }

   strcpy(list_c[i].role, aux);

    //int id_user = see_exists(i,aux2);

    /*if(id_user == -1){
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
    }*/
}

int pushClient(int connfd, char* c_ip, int c_port){    

    int i;

    for(i=0;i<MAX_CLIENT;i++){
        if(list_c[i].socket_num==INVALID_SOCK){
            list_c[i].socket_num = connfd;
            strcpy(list_c[i].ip,c_ip);
            list_c[i].port=c_port;
            strcpy(list_c[i].role,"user");
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
                            printf("%s has been connected from %s \n", list_c[i].nick, list_c[i].ip);
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
                    sprintf(buf1,"MSSG server :> novo utilizador <%s> \n", list_c[i].nick);
                    write(list_c[j].socket_num,buf1,strlen(buf1));
                }
            }
        }
    }else if(aux2 == 0){
       for(int j = 0; j < MAX_CLIENT; j++){  
            if(list_c[j].socket_num!=INVALID_SOCK){
                if(j!=i){
                    sprintf(buf1,"MSSG server :> <%s> mudou o seu nome para <%s> \n", old_nick, list_c[i].nick);
                    write(list_c[j].socket_num,buf1,strlen(buf1));
                }
            }
        }
    }
}


//Authenticate function PASS <password>
void authenticate1(char* chatData, int i){
    int j;
    char* token;
    char buf1[MAXLINE];
    int contador = 0;
    char aux2[20];
    char* end = " ";
    int tamanho = 0;

    int decisao;

    token=strtok(chatData, end);

    memset(buf1,0,sizeof(buf1));

    char* aux = "operador";
    char* aux3 = "autenticado";

    tamanho = strlen(register_c[i].nick);

    if(tamanho > 0){

        decisao = 1;

        if(strcmp(token, authenticate)==0){
            token = strtok(NULL, end);
            token[strcspn(token, "\n")] = 0;
            strcpy(aux2,token);
        }

        for(j=0; j<MAX_CLIENT;j++){
            if(j==i && list_c[j].socket_num!=INVALID_SOCK){
                if(strcmp(register_c[j].password,aux2) == 0){
                    printf("RPLY 201 - Autenticação com sucesso.\n");
                    sprintf(buf1,"RPLY 201 - Autenticação com sucesso.\n");
                    strcpy(register_c[i].auth,aux3);
                    write(list_c[i].socket_num,buf1,strlen(buf1));
                } else {
                    printf("RPLY 203 - Erro. Password incorreta.\n");
                    sprintf(buf1,"RPLY 203 - Erro. Password incorreta.\n");
                    write(list_c[i].socket_num,buf1,strlen(buf1));
                }
            }
        }

    } else {

        decisao = 2;

        sprintf(buf1,"RPLY 202 - Erro. Nome não está definido (não registado).\n");
        write(list_c[i].socket_num,buf1,strlen(buf1));
        printf("RPLY 202 - Erro. Nome não está definido (não registado).\n");
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
                break;
            } else {
                id = -1;  //não encontra utilizador com o nome pedido
            }
        }
    }

    return id; //retorna o id do utilizador encontrado, se for igual a -1, é porque não encontrou esse utilizador.
}

// QUIT function
void quit_func(int i){
    int j;
    char* token=NULL;
    char buf1[MAXLINE];

    int decisao = 0;

    char* aux = "operador";
    char* aux2 = "user";

    memset(buf1,0,sizeof(buf1));

    for(j=0; j<MAX_CLIENT;j++)
        if(j==i && list_c[j].socket_num!=INVALID_SOCK){

            if(strcmp(list_c[i].role,aux) == 0){
                strcpy(list_c[i].role,aux2);
                sprintf(buf1,"RPLY 901 – %s Deixou de ser operador.\r\n",list_c[i].nick);
                write(list_c[i].socket_num,buf1,strlen(buf1));
                printf("RPLY 901 – %s Deixou de ser operador.\r\n",list_c[i].nick);

                for(int k=0; k<MAX_CLIENT;k++){
                    if(k!=i && list_c[k].socket_num!=INVALID_SOCK){
                            strcpy(list_c[i].role,aux2);
                            sprintf(buf1,"MSSG server :> <%s> deixou de ser operador.\r\n",list_c[i].nick);
                            write(list_c[k].socket_num,buf1,strlen(buf1));
                    }
                }
            } else {
                sprintf(buf1,"RPLY 902 – Erro. Ação não autorizada, utilizador %s não é operador.\r\n",list_c[i].nick);
                write(list_c[i].socket_num,buf1,strlen(buf1));
                printf("RPLY 902 – Erro. Ação não autorizada, utilizador %s não é um operador.\r\n",list_c[i].nick);
            }
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

    int decisao = 0;
    int decisao2 = 0;

    token=strtok(chatData, end);

    char* aux = "operador";
    char* aux4 ="autenticado";

    if(strcmp(list_c[i].role,aux) == 0){  //se cliente for operador

        if(strcmp(register_c[i].auth,aux4) == 0){  //se cliente estiver autenticado

            decisao = 1;

            memset(buf1,0,sizeof(buf1));

            if(strcmp(token, role)==0){
                    token = strtok(NULL, end);
                    token[strcspn(token, "\n")] = 0;
                    strcpy(aux2,token);
            }

            int id_user = see_exists(i,aux2);

            if(id_user == -1){
                sprintf(buf1,"Utilizador pretendido não existe.\n");
                write(list_c[i].socket_num,buf1,strlen(buf1));
            } else {

                if(strcmp(register_c[id_user].nick, aux2) == 0){   //ver se o utilizador está na lista de clientes registados
                    strcpy(list_c[id_user].role, aux);
                    puts("RPLY 801 – Utilizador pretendido foi promovido a operador.\n");
                    sprintf(buf1,"RPLY 801 – Utilizador pretendido foi promovido a operador.\n");
                    write(list_c[i].socket_num,buf1,strlen(buf1));

                    for(j=0; j<MAX_CLIENT;j++){
                         if(j!=i && list_c[j].socket_num!=INVALID_SOCK){
                             
                             sprintf(buf1,"MSSG server :> <%s>  foi promovido a operador\n", aux2);
                             write(list_c[j].socket_num,buf1,strlen(buf1));
                         } 
                    }
                } else {
                    puts("RPLY 804 – Erro. Ação não autorizada, utilizador <%s> não é um utilizador registado.\n");
                    sprintf(buf1,"RPLY 804 – Erro. Ação não autorizada, utilizador <%s> não é um utilizador registado.\n", aux2);
                    write(list_c[i].socket_num,buf1,strlen(buf1));
                }

            }

        } else {
            puts("RPLY 803 – Erro. Ação não autorizada, utilizador cliente não está autenticado.\n");
            sprintf(buf1,"RPLY 803 – Erro. Ação não autorizada, utilizador cliente não está autenticado.\n");
            write(list_c[i].socket_num,buf1,strlen(buf1));
        }


    } else {
        decisao = 2;
        puts("RPLY 802 – Erro. Ação não autorizada, utilizador cliente não é um operador.\n");
        sprintf(buf1,"RPLY 802 – Erro. Ação não autorizada, utilizador cliente não é um operador.\n");
        write(list_c[i].socket_num,buf1,strlen(buf1));
    }
}

//Give user a password
void give_password(int i, char* password){
    int j,cnt=0;
    char buf1[MAXLINE];

    int id = 0;

    memset(buf1,0,sizeof(buf1));

    strcpy(register_c[i].password, password);
    strcpy(register_c[i].nick, list_c[i].nick);

    sprintf(buf1,"RPLY 701 – Utilizador foi registado com sucesso.\n");
    write(list_c[i].socket_num,buf1,strlen(buf1));
    printf("RPLY 701 – Utilizador foi registado com sucesso.\n");

}

//List of people registered
void list_registered(int i){
    int j,cnt=0;
    char buf1[MAXLINE];

    memset(buf1,0,sizeof(buf1));

    for(j=0; j<MAX_CLIENT;j++){
        int tamanho = strlen(register_c[j].nick);
        if(tamanho > 0){
            cnt++;
            sprintf(buf1,"[Utilizador registado : %s, %s, %s]\n",register_c[j].nick, register_c[j].password, register_c[i].auth);
            write(list_c[i].socket_num,buf1,strlen(buf1));
        }
    }

    sprintf(buf1,"[Nº de utilizadores registados : %d]\n",cnt);
    write(list_c[i].socket_num,buf1,strlen(buf1));        
}

//Register user in list of registed users
void user_register(char* chatData, int i){
    int j;
    char* token=NULL;
    char buf1[MAXLINE];
    int contador = 0;
    char* aux2[20];
    char* aux3[20];
    char* pass[20];

    int decisao1 = 0;

    int auxiliar = 0;

    char* end = " ";

    token=strtok(chatData, end);

    char* aux = "operador";
    char* autenticacao = "não_autenticado";
    //strcpy(aux,"OPERADOR");

    memset(buf1,0,sizeof(buf1));

    if(strcmp(list_c[i].role,aux) == 0){  //se utilizador for operador

        if(strcmp(token, register1) ==0){
            token = strtok(NULL, end);
            strcpy(aux2,token);
            token = strtok(NULL, end);
            token[strcspn(token, "\n")] = 0;
            strcpy(aux3,token);
        }

        for(j=0; j<MAX_CLIENT;j++){
            if(strcmp(list_c[j].nick,aux2) == 0){
                give_password(j,aux3);
                strcpy(register_c[j].auth, autenticacao);
                auxiliar = j;
                decisao1 = 2;
                break;
            } else {
                decisao1 = 1;
            }
        }

        if(decisao1 == 1){
            sprintf(buf1,"Erro. Ação não autorizada, nickname mal introduzido (ou não existe na lista de registados).\n");
            write(list_c[i].socket_num,buf1,strlen(buf1));
            puts("Erro. Ação não autorizada, nickname mal introduzido (ou não existe na lista de registados).\n");
        } else if(decisao1 == 2){
            for(j=0; j<MAX_CLIENT;j++){
                if(j!=i && list_c[j].socket_num!=INVALID_SOCK){
                    sprintf(buf1,"MSSG server :> <%s> foi registado.\n", list_c[auxiliar].nick);
                    write(list_c[j].socket_num,buf1,strlen(buf1));
                }
            }
        }

    } else {
        sprintf(buf1,"RPLY 702 – Erro. Ação não autorizada, utilizador cliente não é um operador.\n");
        write(list_c[i].socket_num,buf1,strlen(buf1));
        puts("RPLY 702 – Erro. Ação não autorizada, utilizador cliente não é um operador.\n");
    }
}

//KICK from register list
void kick_user(char* chatData, int i){
    int j;
    char* token=NULL;
    char buf1[MAXLINE];
    char* aux2[20];

    int decisao = 0;

    char* end = " ";

    token=strtok(chatData, end);

    char* aux = "operador";

    memset(buf1,0,sizeof(buf1));

    if(strcmp(list_c[i].role,aux) == 0){

        if(strcmp(token, kick) ==0){
            token = strtok(NULL, end);
            token[strcspn(token, "\n")] = 0;
            strcpy(aux2,token);
        }

        remove_register(i, aux2);

    } else {
        puts("RPLY 602 – Erro. Ação não autorizada, utilizador cliente não é um operador.\n");
        sprintf(buf1,"RPLY 602 – Erro. Ação não autorizada, utilizador cliente não é um operador.\n");
        write(list_c[i].socket_num,buf1,strlen(buf1));
    }

}
//Remove register user
void remove_register(int i,char* user){
    int j;
    char buf1[MAXLINE];

    char* aux="";
    char* role="operador";

    char* aux4="user";

    int cnt;

    int id = 0;

    int decisao = 0;

    memset(buf1,0,sizeof(buf1));

    if(strcmp(list_c[i].role,role) == 0){

        for(j=0; j<MAX_CLIENT;j++){

            if(strcmp(register_c[j].nick,user) == 0){
                strcpy(register_c[j].nick,aux);
                strcpy(register_c[j].password,aux);
                strcpy(register_c[j].role,aux4);

                sprintf(buf1,"RPLY 601 – Utilizador expulso.\n");
                write(list_c[i].socket_num,buf1,strlen(buf1));
                decisao = 2;
                break;

                //dar user role ainda
            } else {
                decisao = 1;
            }
        }

        if(decisao == 1){
            sprintf(buf1," Utilizador que quer expulsar não está registado (ou não existe).\n");
            write(list_c[i].socket_num,buf1,strlen(buf1));
        }

    } else {
        sprintf(buf1,"\nRPLY 602 – Erro. Ação não autorizada, utilizador %s não é um operador.", list_c[i].nick);
        write(list_c[i].socket_num,buf1,strlen(buf1));
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
    sprintf(buf1,"[Visitantes atuais do canal : %d]\n",cnt);
    write(list_c[i].socket_num,buf1,strlen(buf1));

    for(j=0; j<MAX_CLIENT;j++){
        if(list_c[j].socket_num!=INVALID_SOCK){
            sprintf(buf1,"[%s from %s: %d]\n",list_c[j].nick,list_c[j].ip,list_c[j].port);
            write(list_c[i].socket_num,buf1,strlen(buf1));
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


    char* end = " ";
    char* token=NULL;
    char buf1[MAXLINE];
    char buf2[MAXLINE];
    char chatData[CHATDATA];

    /*char* aux = "operador";
    char* aux1 = "ADMIN";

    strcpy(list_c[9].role,aux);
    strcpy(list_c[9].nick,aux1);

    strcpy(register_c[9].role,aux);
    strcpy(register_c[9].nick, aux1);*/

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

    char *message2 = "Session started! \n set your nickname with command NICK first!\n";


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

                message2 = "Welcome! \n Set your nickname with command NICK first! --> NICK <nickname>  \n";

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
                        sprintf(buf2,"NICK COMMMAND SHOLUD BE FIRST \n");
                        write(list_c[j].socket_num,buf2,strlen(buf2));
                        continue;

                    }else{
                         // Encerrando a conexão [/ quit]
                        if(!strcmp(chatData, quit)){   // disconnect from the client "i"
                            quit_func(i);
                            //popClient(list_c[i].socket_num);
                            continue;
                        }

                        // Client Mostrar lista [/ list]
                        if(!strcmp(chatData,whos)){ //print the list of clients
                            whos_func(i);
                            continue;
                        }

                        //Client register
                        if(strstr(chatData, register1) != NULL){
                            user_register(chatData,i);
                            continue;
                        }
                        //List of registered users
                        if(!strcmp(chatData,list_regs)){
                            list_registered(i);
                            continue;
                        }

                        //Authenticated users
                        if(strstr(chatData, authenticate) != NULL){
                            authenticate1(chatData,i);
                            continue;
                        }

                        //Kick user from being operator
                        if(strstr(chatData, kick) != NULL){
                            kick_user(chatData, i);
                            continue;
                        }
                      /*  //Enviar mensagem 1:1 [/ mensagem [Cliente] [mensagem]]
                        if(strstr(chatData, message) != NULL){
                            if(message_func(chatData, i) == 3){
                                continue;
                            }
                        }
*/
                        // Gives the user is information
                        if(!strcmp(chatData,user_info)){
                            show_client_info(i);
                            continue;
                        }

                        if(strstr(chatData, role) != NULL){
                            give_role(chatData, i);
                            continue;
                        }
                        //admin operator
                        if(strstr(chatData, admin) != NULL){
                            give_admin_role(chatData, i);
                            continue;
                        }

                    }
                }
            }
        }
    }
}