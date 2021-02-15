/** TCP CLIENT **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>

#define CHATDATA 512

// Command
char ERROR[]= "Erro.\n";
char quit[]="QUIT\n";
char whos[]="WHOS\n";
char message[]="MSSG";
char nickname[]="NICK";
char role[]="OPER";
char user_info[]="INFO\n";
char message2[]="MAIN";
char exists[]="EXISTS\n";
char kick[]="KICK";
char register1[]="REGS";
char list_register[]="LISTR\n";
char authenticate[]="PASS";
char admin[]="ADMI";

void
chatting(int sockfd, int maxfdp, fd_set rset, char *argv[])
{
    char chatData[CHATDATA];	// Chatting message
    char buf[CHATDATA];			// Command message
    int n;						// Length of buffer for using read()

    while(1){
        FD_ZERO(&rset);			// fd_set Variável de inicialização (rset)
        FD_SET(0,&rset);		// Stdin Agrupamento
        FD_SET(sockfd, &rset);	// Socket Agrupamento

        // Verificação de mudança do descritor de arquivo
        if(select(maxfdp, &rset, (fd_set *)0, (fd_set *)0, (struct timeval *)0) <0) {
            perror("select");
            exit(1);
        }

        /************** Imprimir mensagens de outros clientes ***************/

        // Verificar se há um socket em FD_SET
        if(FD_ISSET(sockfd,&rset))
        {
            memset(chatData, 0, sizeof(chatData));	// Inicializar chatData
            
            // Ler as mensagens de bate-papo do descritor de arquivo Socket.
            if((n = read(sockfd, chatData, sizeof(chatData))) > 0)
            {

                write(1, chatData, n); // A mensagem lida é exibida na tela do cliente.
            }
        }

        /*************************************************************/


        /*************** Enviar sua própria mensagem para o servidor ***************/

        if(FD_ISSET(0, &rset))
        {
            memset(buf, 0, sizeof(buf));	//	inicializar buf 

            if((n = read(0, buf, sizeof(buf))) > 0 )
            {     

                // Verifique a entrada do comando, se for um comando, gravar o comando em sockfd.

                if(strstr(buf, nickname) != NULL)
                {
                    write(sockfd, buf, strlen(buf));
                    continue;
                }
                if(strstr(buf, authenticate) != NULL)
                {
                    write(sockfd, buf, strlen(buf));
                    continue;
                }
                if(!strcmp(buf, quit))
                {
                    write(sockfd, buf, strlen(buf));
                    //break;
                    continue;
                }
                if(!strcmp(buf,whos))
                {
                    write(sockfd, buf, strlen(buf));
                    continue;
                }
                //List of registered clients
                if(!strcmp(buf,list_register))
                {
                    write(sockfd, buf, strlen(buf));
                    continue;
                }
                if(!strcmp(buf,user_info))
                {
                    write(sockfd, buf, strlen(buf));
                    continue;
                }
                if(strstr(buf,register1) != NULL)
                {
                    write(sockfd, buf, strlen(buf));
                    continue;
                }
                if(strstr(buf,kick) != NULL)
                {
                    write(sockfd, buf, strlen(buf));
                    continue;
                }
                if(!strcmp(buf,exists))
                {
                    write(sockfd, buf, strlen(buf));
                    continue;
                }
                if(strstr(buf,message) != NULL)
                {
                    write(sockfd, buf, strlen(buf));
                    continue;
                }
                if(strstr(buf,role))
                {
                    write(sockfd, buf, strlen(buf));
                    continue;
                }
                if(strstr(buf,admin))
                {
                    write(sockfd, buf, strlen(buf));
                    continue;
                }

                /*
                write(sockfd, chatData, strlen(chatData));
                */

            }
        }

    }
}

void
main(int argc,char *argv[])
{
    int sockfd;						// Socket file descriptor
    struct sockaddr_in servaddr;	// Server ip address
    int maxfdp;						// Max file descriptor
    fd_set rset;

    // Saída de mensagem de erro quando a entrada do parâmetro está incorreta
    if(argc < 3)
    {
        printf("usage:%s [ip_address] [port_number]\n",argv[0]);
        exit(0);
    }

    // IPv4, criação de socket TCP / IP
    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
        perror("socket");
        exit(1);
    };

    memset(&servaddr, 0, sizeof(servaddr));			
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);	
    servaddr.sin_family=AF_INET;					
    servaddr.sin_port=htons(atoi(argv[2]));		

    // Quando a solicitação de conexão com o servidor falha, uma mensagem de erro é exibida e, em seguida, encerrada.
    if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        perror("connect");
        exit(0);
    }

    maxfdp=sockfd + 1;

    // Execução da função de bate-papo
    chatting(sockfd, maxfdp, rset, argv);

    // Close socket
    close(sockfd);
}