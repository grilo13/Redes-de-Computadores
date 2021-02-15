#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>

#define CHATDATA 512    //tamanho máximo por mensagem

// Comandos usados
char ERROR[]= "Erro.\n";
char quit[]="QUIT\n";
char whos[]="WHOS\n";
char message[]="MSSG";
char nickname[]="NICK";
char role[]="OPER";
char user_info[]="INFO\n";
char exists[]="EXISTS\n";
char kick[]="KICK";
char register1[]="REGS";
char list_register[]="LISTR\n";
char authenticate[]="PASS";
char admin[]="ADMI";
char exit_chat[]="EXIT\n";

void chatting(int sockfd, int maxfdp, fd_set rset, char *argv[]){
    char chatData[CHATDATA];	// array para a mensagem
    char buf[CHATDATA];			// auxixiliar que será comparado com o input do utilizador
    int n;						// valor do read

    while(1){
        FD_ZERO(&rset);			// fd_set Variável de inicialização (rset)
        FD_SET(0,&rset);		
        FD_SET(sockfd, &rset);	// Socket s

        // verificação de mudança de cliente
        if(select(maxfdp, &rset, (fd_set *)0, (fd_set *)0, (struct timeval *)0) <0) {
            perror("select");
            exit(1);
        }

        /************** Imprimir mensagens de outros clientes ***************/

        // Verificar se há um socket em FD_SET
        if(FD_ISSET(sockfd,&rset))
        {
            memset(chatData, 0, sizeof(chatData));	// Inicializar chatData, array da mensagem
            
            // ler as mensagens do chat
            if((n = read(sockfd, chatData, sizeof(chatData))) > 0)
            {

                write(1, chatData, n);  //escreve a mensagem na tela do cliente
            }
        }

        /*************** Enviar a sua mensagem para o servidor ***************/

        if(FD_ISSET(0, &rset))
        {
            memset(buf, 0, sizeof(buf));	//	inicializar buf 

            if((n = read(0, buf, sizeof(buf))) > 0 )
            {     

                // Verificar a entrada do comando, se for um comando, gravar o comando em sockfd. (um comando existente)

                //nickname = adiciona o nickname ao cliente, NICK
                if(strstr(buf, nickname) != NULL)
                {
                    write(sockfd, buf, strlen(buf));
                    continue;
                }

                //authentica = autentica um cliente, mediante as condições pedidas, = PASS
                if(strstr(buf, authenticate) != NULL)
                {
                    write(sockfd, buf, strlen(buf));
                    continue;
                }

                //quit = retira a categoria "operador" de um cliente, = QUIT
                if(!strcmp(buf, quit))
                {
                    write(sockfd, buf, strlen(buf));
                    continue;
                }

                //whos = lista e mostra a informação dos utilizadores ativos no canal = WHOS
                if(!strcmp(buf,whos))
                {
                    write(sockfd, buf, strlen(buf));
                    continue;
                }

                //list_register = lista e mostra a informação dos utilizadores registados no canal = LISTR
                if(!strcmp(buf,list_register))
                {
                    write(sockfd, buf, strlen(buf));
                    continue;
                }

                //user_info = mostra a informação do cliente (nick, categoria e ip) = INFO
                if(!strcmp(buf,user_info))
                {
                    write(sockfd, buf, strlen(buf));
                    continue;
                }

                //register1 =  regista um utilizador (atribui-lhe nick e password, se ele existir, e mediante condições pedidas) = REGS
                if(strstr(buf,register1) != NULL)
                {
                    write(sockfd, buf, strlen(buf));
                    continue;
                }

                //kick = remove dos utilizadores registados um cliente (se o mesmo lá estiver) = KICK
                if(strstr(buf,kick) != NULL)
                {
                    write(sockfd, buf, strlen(buf));
                    continue;
                }

                //exists = retorna o id de um utilizador listado, ou -1 se não o encontra = EXISTS
                if(!strcmp(buf,exists))
                {
                    write(sockfd, buf, strlen(buf));
                    continue;
                }

                //exit_chat = utilizador sai do canal onde está= EXIT
                if(!strcmp(buf,exit_chat))
                {
                    write(sockfd, buf, strlen(buf));
                    break;
                }

                //message = troca de mensagens entre os utilizadores, MSSG
                if(strstr(buf,message) != NULL)
                {
                    write(sockfd, buf, strlen(buf));
                    continue;
                }

                //role = permite atribuir a categoria OPERADOR a um utilizador registado (apenas um operador o pode fazer) = OPER
                if(strstr(buf,role))
                {
                    write(sockfd, buf, strlen(buf));
                    continue;
                }

                //admin = apenas usado pelo admin, com o intuito de atribuir o nick, pass e autenticar automaticamente o admin = ADMI
                if(strstr(buf,admin) != NULL)
                {
                    write(sockfd, buf, strlen(buf));
                    continue;
                }
            }
        }
    }
}

void
main(int argc,char *argv[])
{
    int sockfd;						
    struct sockaddr_in servaddr;	// recebe informações do servidor
    int maxfdp;						
    fd_set rset;

    // Saída de mensagem de erro quando a entrada do parâmetro está incorreta
    if(argc < 3)
    {
        printf("usage:%s [ip_address] [port_number]\n",argv[0]);
        exit(0);
    }

    // criação da socket TCP / IP
    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
        perror("socket");
        exit(1);
    };

    memset(&servaddr, 0, sizeof(servaddr));   //limpa a struct com as infos do servidor
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);	
    servaddr.sin_family=AF_INET;	//ipv4				
    servaddr.sin_port=htons(atoi(argv[2]));	//port number, é dada logo no input do client ./client PORT

    // se a conexão com o servidor falhar, diz que existe um erro (faz a conexão entre cliente e servidor)
    if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        perror("connect");
        exit(0);
    }

    maxfdp=sockfd + 1;

    // função que permite a troca de mensagens entre os clientes (e o servidor)
    chatting(sockfd, maxfdp, rset, argv);

    // fecha a socket
    close(sockfd);
}