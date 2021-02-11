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

#define MAXLINE 100
#define MAX_CLIENT 1000
#define CHATDATA 512
#define INVALID_SOCK -1
#define ROLESIZE 10

char greeting[]="Bem-vindo à sala.\n------------> Menu <------------\nUsages:\nQUIT to leave the channel\nLIST to see the users in the page\n\n";
char ERROR[]= "Erro.\n";
char quit[]="QUIT\n";
char list[]="LIST\n";
char message[]="MSSG";
char nickname[] ="NICK\n";
char role[]="ROLE\n";
char user_info[]="INFO\n";
char message2[]="MAIN";

// Client List 정보
struct List_c{
    int socket_num;         // Socket number
    char nick[CHATDATA];    // Client nickname 
    char username[CHATDATA];
    char ip[40];            // Client IP 
    char role[ROLESIZE];    // role of the user
    int port;               // Server port 
}list_c[MAX_CLIENT];

// 접속한 클라이언트를 인덱스 저장
int pushClient(int connfd, char* c_nick, char* c_ip, int c_port){
    int i;

    for(i=0;i<MAX_CLIENT;i++){
        if(list_c[i].socket_num==INVALID_SOCK){
            list_c[i].socket_num = connfd;
            strcpy(list_c[i].nick,c_nick);
            strcpy(list_c[i].ip,c_ip);
            //strcpy(list_c[i].role,c_role);
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

    for(i =0; i<MAX_CLIENT;i++){
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
    printf("%s logged out from the ip %s\r\n",list_c[i].nick, list_c[i].ip);
    for(j=0; j<MAX_CLIENT;j++)
        if(j!=i && list_c[j].socket_num!=INVALID_SOCK){
            sprintf(buf1,"%s Deixou de ser operador.\r\n",list_c[i].nick);
            write(list_c[j].socket_num,buf1,strlen(buf1));
        }
}

//give role to user
void give_role(int i){
    int j;
    char* token=NULL;
    char buf1[MAXLINE];

    memset(buf1,0,sizeof(buf1));
    printf("%s\n", list_c[i].role);
    printf("%s wants to add a role\r\n",list_c[i].nick);

    //list_c[i].role = "USER";
    strcpy(list_c[i].role, "USER");

    printf("%s\n", list_c[i].role);
    for(j=0; j<MAX_CLIENT;j++)
        if(j!=i && list_c[j].socket_num!=INVALID_SOCK){
            sprintf(buf1,"%s is now a %s\r\n",list_c[i].nick, list_c[i].role);
            write(list_c[j].socket_num,buf1,strlen(buf1));
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
            sprintf(buf1,"User %d details\n-> %s\n-> %s\n-> %s\n-> %s\r\n",list_c[i].socket_num ,list_c[i].nick, list_c[i].role, list_c[i].ip,list_c[i].username);
            write(list_c[j].socket_num,buf1,strlen(buf1));
        }
}

//unknown command



//add username to a user
void username(int i){
    int j;
    char* token=NULL;
    char* aux="pedrinho13";
    char buf1[MAXLINE];
    int contador = 0;
    int tamanho = 0;

    memset(buf1,0,sizeof(buf1));
    printf("%s\n", list_c[i].username);

    //list_c[i].role = "USER";
    //strcpy(list_c[i].username, aux);

    //printf("%s\n", list_c[i].username);

    //int result = strcmp(list_c[j].username,aux);

    //printf("Contador %d\n", contador);

    for(j=0; j<MAX_CLIENT;j++){
        if(strcmp(list_c[j].username,aux) == 0){   //if Return value = 0 then it indicates str1 is equal to str2.
            contador++;
        }
    }
    tamanho = strlen(list_c[i].username);

    printf("Tamanho = %d\n", tamanho);

    if(tamanho > 0){
        sprintf(buf1,"%s you already have an username.\r\n",list_c[i].nick);
        write(list_c[j].socket_num,buf1,strlen(buf1));
    }

    if(contador == 0){
        strcpy(list_c[i].username, aux);
        printf("%s add the username\r\n",list_c[i].nick);
    }

    if(contador > 0){
        printf("Theres alreay a user with the username. Please retry again %s\n", aux);
    }

    for(j=0; j<MAX_CLIENT;j++)
        if(j!=i && list_c[j].socket_num!=INVALID_SOCK){
                if(contador == 0){
                    sprintf(buf1,"%s set the username: %s\r\n",list_c[i].nick, list_c[i].username);
                    write(list_c[j].socket_num,buf1,strlen(buf1));
                } else {
                    sprintf(buf1,"%s tried to change is username.\r\n",list_c[i].nick);
                    write(list_c[j].socket_num,buf1,strlen(buf1));
                }
        }
        else if(j==i && list_c[j].socket_num!=INVALID_SOCK){
            if(contador == 0){
                    sprintf(buf1,"%s, this is your new username: %s\r\n",list_c[i].nick, list_c[i].username);
                    write(list_c[j].socket_num,buf1,strlen(buf1));
            } else {
                    sprintf(buf1,"%s your username is already being used.\r\n",list_c[i].nick);
                    write(list_c[j].socket_num,buf1,strlen(buf1));
            }
        }
}

//change_nickname
void change_nickname(int i){   //change the nickname of the user with index i
    int j;
    char* token=NULL;
    char* aux = NULL;
    char* aux2 = NULL;
    char buf1[MAXLINE];
    char buf2[MAXLINE];

    memset(buf1,0,sizeof(buf1));
    memset(buf2,0,sizeof(buf2));

    aux = list_c[i].nick;
    //printf("%s\n", aux);
    //printf("%s\n",list_c[i].nick);
    //CHANGE_STR( &aux, "pedro" );
    //CHANGE_STR(&list_c[i].nick, "joao");

    aux = list_c[i].nick;
    //aux2 = list_c[i].role;

    printf("User has nick %s and role %s", aux,aux2);
    //printf("%s", aux);
    //printf("%s", list_c[i].nick);

    //list_c[i].nick = aux2;
    
    printf("%s is changed is nickname to .... %s\r\n",list_c[i].nick,aux, list_c[i].ip);

    for(j=0; j<MAX_CLIENT;j++)

        if(j!=i && list_c[j].socket_num!=INVALID_SOCK){

            sprintf(buf1,"%s Mudou o seu nickname.\r\n",list_c[i].nick);

            write(list_c[j].socket_num,buf1,strlen(buf1));
        }

}

//Function to change the value of a string (used in changing the nickname)
void CHANGE_STR(char **old_str, char *new_str)   {
    *old_str = new_str;    
}
   
// list
void list_func(int i){
    int j,cnt=0;
    char buf1[MAXLINE];
    char buf2[MAXLINE];

    memset(buf2,0,sizeof(buf2));
    memset(buf1,0,sizeof(buf1));
    for(j=0; j<MAX_CLIENT;j++)
        if(list_c[j].socket_num!=INVALID_SOCK)cnt++;
    sprintf(buf1,"[Visitante atual : %d]\r\n",cnt);
    write(list_c[i].socket_num,buf1,strlen(buf1));
    printf("%s asked for the list of users.\n", list_c[i].nick);
    for(j=0; j<MAX_CLIENT;j++)
        if(j == i &&  list_c[j].socket_num!=INVALID_SOCK){
            sprintf(buf1,"[%s from %s: %d]\r\n",list_c[j].nick,list_c[j].ip,list_c[j].port);
            write(list_c[i].socket_num,buf1,strlen(buf1));
            
        }

        /*if(j!=i && list_c[j].socket_num!=INVALID_SOCK){
            sprintf(buf2,"[%s asked for a list of the users %s: %d]\r\n",list_c[i].nick,list_c[j].ip,list_c[j].port);
            write(list_c[j].socket_num,buf2,strlen(buf2));
        }*/
}
// message
int message_func(char* chatData,int i){
    int j,message_sock;
    char* token=NULL;
    char buf1[MAXLINE];
    char* aux = NULL;

    memset(buf1,0,sizeof(buf1));
    token=strtok(chatData, " ");
    char * end;

    if(strcmp(token, message)){
        if((end=strtok(NULL,"\n"))==NULL){
            //printf("%s The message is empty", list_c[i].nick);
            sprintf(buf1,"%s",token);
        }
        else {
            sprintf(buf1,"%s %s",token,end);
        } 
        sprintf(chatData,"[%s] send the message %s\r\n",list_c[i].nick,buf1);
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
//[/message [Client][message]]
//quero fazer [/nick [nickname]]

int message2_func(char* chatData,int i){
    int j,message_sock;
    char* token=NULL;
    char buf1[MAXLINE];
    char* aux = NULL;  //dar a auxiliar o valor do nickname existente, para puder fazer a resposta do servidor usando o nome antigo
    char* aux2 = NULL;
    int contador = 0;
    int indice = 1;
    int tamanho = 0;
    char array[MAXLINE];
    int decisao = 0;  //usado para ver se já existe o utilizador já tem um nickname anterior, e quer mudar para outro

    memset(buf1,0,sizeof(buf1));
    token=strtok(chatData, " ");
    char * end;

    tamanho = strlen(list_c[i].username);

    printf("Tamanho %d\n", tamanho);

    aux = list_c[i].username; //atribuir a aux o valor do username

    for(j=0; j<MAX_CLIENT;j++){
        if((strcmp(list_c[j].username, list_c[i].username) == 0)){ 
            printf("J-> %s",list_c[j].username );
            printf("I-> %s",list_c[i].username );
        }
    }


    if(tamanho > 0){

        decisao = 1;

        printf("Já tem um nickname atribuido, vai alterar o mesmo %s", list_c[i].nick);

    } else {

        decisao = 2;

        printf("Vai inserir um nickaname %s", list_c[i].nick);
    }
   
    /* walk through other tokens */
    while( token != NULL ) {
      contador++;
      printf( " %s\n", token );
      printf("%d\n", contador);
      indice++;

      if(contador == 3){
          strcpy(list_c[i].username,token);
      }
    
      token = strtok(NULL, " ");
   }
   printf("%s", list_c[i].username);

   printf("AUX -> %s\n", aux);

   if(decisao == 2){   //quando o utilizador está a criar o nickname pela primeira vez
        for(j=0; j<MAX_CLIENT;j++)

        if(j!=i && list_c[j].socket_num!=INVALID_SOCK){

            sprintf(buf1,"%s Server -> Novo utilizador %s\r\n",list_c[i].nick, list_c[i].username);

            write(list_c[j].socket_num,buf1,strlen(buf1));
        } else if(j==i && list_c[j].socket_num!=INVALID_SOCK){

            sprintf(buf1,"%s Nome atribuido com sucesso. %s\r\n",list_c[i].nick, list_c[i].username);

            write(list_c[j].socket_num,buf1,strlen(buf1));
        }
   } else if(decisao == 1){

       for(j=0; j<MAX_CLIENT;j++)

            if(j!=i && list_c[j].socket_num!=INVALID_SOCK){

                sprintf(buf1,"%s mudou o seu nickname para %s\r\n",/*list_c[i].nick*/aux, list_c[i].username);

                write(list_c[j].socket_num,buf1,strlen(buf1));
            } else if(j==i && list_c[j].socket_num!=INVALID_SOCK){

                sprintf(buf1,"%s alterou o seu nome com sucesso para %s\r\n",/*list_c[i].nick*/aux, list_c[i].username);

                write(list_c[j].socket_num,buf1,strlen(buf1));
            }
   }
}

void main(int argc, char *argv[])
{
    int newSockfd,sockfd;                      // Socket File Descriptor
    struct sockaddr_in servaddr,cliaddr;
    int clilen;
    int maxfd=0;                            // 입출력 다중화에서 관리하는 파일의 갯수
    int i,j,n;
    fd_set rset;

    int index;

    char* token=NULL;
    char buf1[MAXLINE];
    char buf2[MAXLINE];
    char buf3[MAXLINE];
    char chatData[CHATDATA];

    // ./server [port_number] 인자를 안넣으면 메세지 뿌리기
    if(argc<2){
        printf("usage: %s port_number\n",argv[0]);
        exit(-1);
    }

    // Socket 생성
    // SOCK_STREAM : TCP/IP 프로토콜
    if ((sockfd=socket(AF_INET,SOCK_STREAM,0)) < 0){
        perror("socket");
        exit(1);
    }

    // socket에 필요한 정보
    memset(&servaddr,0,sizeof(servaddr));       // servaddr 크기만큼 0x00으로 초기화
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY); // INADDR_ANY : 자기 자신의 IP주소가 들어간다. 어떤 IP 주소라도 매핑을 시켜라라는 의미
    servaddr.sin_family=AF_INET;                // IPv4 인터넷 프로토콜
    servaddr.sin_port=htons(atoi(argv[1]));     // network byte로 바꿔야 하므 htons 사용

    // socket에 필요한 정보를 커널에 등록
    if(bind(sockfd,(struct sockaddr *) &servaddr,sizeof(servaddr)) < 0){
        printf("Error : Kill process.\n");
        exit(1);
    }

    // Client 접속 요청 확인. 형식적인 것..
    if(listen(sockfd, MAX_CLIENT) < 0){
        printf("listen");
        exit(1);
    }

    // socket_num 초기화
    for(i=0;i<MAX_CLIENT;i++)
        list_c[i].socket_num=INVALID_SOCK;

    // buf1 초기화
    memset(buf1,0,sizeof(buf1));

    //2진수 IP주소를 10진수 IP주소로 변환해서 buf1에 저장
    inet_ntop(AF_INET, &servaddr.sin_addr, buf1, sizeof(buf1));
    printf("[Server address %s : %d]\n", buf1, ntohs(servaddr.sin_port));


    for ( ; ; )
    {
        maxfd=sockfd;

        FD_ZERO(&rset);              // rset 초기화
        FD_SET(sockfd, &rset);       // select 방식 이용
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

        // FD_ISSET : 어떤 스타일로 Socket이 왔느냐 ..
        if(FD_ISSET(sockfd, &rset)){
            clilen=sizeof(cliaddr);
            // client와 connection이 맺어지기를 기다린다.
            // accept가 되면 newSockfd라는 새로운 socket 하나 더 생성. newSockfd를 이용해 데이터 주고 받음.
            if((newSockfd=accept(sockfd, (struct sockaddr *)&cliaddr, &clilen)) > 0) {
                memset(buf1, 0, sizeof(buf1));
                memset(buf2, 0, sizeof(buf2));
                read(newSockfd, buf1, sizeof(buf1));   // Client의 닉네임을 읽는다.
                inet_ntop(AF_INET, &cliaddr.sin_addr, buf2, sizeof(buf2));
                index = pushClient(newSockfd, buf1, buf2, ntohs(cliaddr.sin_port));
                // Client의 sock_num, nick, ip, port_num 삽입
                printf("%s has been connected from %s\n", list_c[index].nick, list_c[index].ip );

                if(index < 0){
                    write(newSockfd,ERROR,strlen(ERROR));
                    close(newSockfd);
                }else{

                    write(newSockfd, greeting, strlen(greeting));
                    for(i=0; i<MAX_CLIENT;i++)
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

                    // 기본 채팅 메세지 보내기
                    for(j=0; j<MAX_CLIENT;j++){   // send chatting letters
                        if(list_c[i].socket_num !=INVALID_SOCK)
                            if(j!=i)
                                write(list_c[j].socket_num, chatData, sizeof(chatData));
                    }
                    // 접속 종료하기 [/quit]
                    if(!strcmp(chatData, quit)){   // disconnect from the client "i"
                        quit_func(i);
                        popClient(list_c[i].socket_num);
                        continue;
                    }
                    // Client 리스트 보여주기 [/list]
                    if(!strcmp(chatData,list)){//print the list of clients
                        list_func(i);
                        continue;
                    }
                    // Change nickname of the client [/nick]
                    if(!strcmp(chatData,nickname)){
                        username(i);
                        continue;
                    }
                    // Gives the user is information
                    if(!strcmp(chatData,user_info)){
                        show_client_info(i);
                        continue;
                    }
                    // Change nickname of the client [/nick]
                    if(!strcmp(chatData,role)){
                        give_role(i);
                        continue;
                    }
                    // 1:1 메세지 보내기 [/message [Client][message]]
                    if(strstr(chatData, message) != NULL){
                        if(message_func(chatData, i) == 0) continue;
                        else;
                    }
                    if(strstr(chatData, message2) != NULL){
                        if(message2_func(chatData, i) == 0) continue;
                        else;
                    }
                }
            }
        }

    }
}