/** TCP CLIENT **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>

#define CHATDATA 1024

// Command
char quit[]="QUIT\n";
char list[]="LIST\n";
char message[]="MSSG";
char message2[]="MSSG2";
char nickname[]="NICK\n";
char kick[]="KICK\n";
char role[]="ROLE\n";
char info[]="INFO\n";
char exists[]="EXISTS\n";

void
chatting(int sockfd, int maxfdp, fd_set rset, char *argv[])
{
    char chatData[CHATDATA];	// Chatting message
    char buf[CHATDATA];			// Command message
    int n;						// Length of buffer for using read()

    while(1){
        FD_ZERO(&rset);			// fd_set 변수(rset) 초기화
        FD_SET(0,&rset);		// Stdin 그룹화
        FD_SET(sockfd, &rset);	// Socket 그룹화

        // 파일디스크립터 변화 확인
        if(select(maxfdp, &rset, (fd_set *)0, (fd_set *)0, (struct timeval *)0) <0) {
            perror("select");
            exit(1);
        }

        /************** 다른 클라이언트의 메세지 출력 ***************/

        // FD_SET에 socket이 있는 지 확인
        if(FD_ISSET(sockfd,&rset))
        {
            memset(chatData, 0, sizeof(chatData));	// chatData 초기화
            // Socket 파일 디스크립터에서 채팅 메세지를 읽어온다.
            if((n = read(sockfd, chatData, sizeof(chatData))) > 0)
            {
                // 읽어온 메세지를 클라이언트 화면에 출력한다.
                write(1, chatData, n);
            }
        }
        /*************************************************************/


        /*************** Transferir mensagem para o server ***************/

        // FD_SET에 stdin이 있는 지 확인
        if(FD_ISSET(0, &rset))
        {
            memset(buf, 0, sizeof(buf));	//	buf 초기화

            if((n = read(0, buf, sizeof(buf))) > 0 )
            {
                // 명령어 입력 확인, 명령어인 경우 명령어를 sockfd에 쓴다.
                if(!strcmp(buf, quit))
                {
                    write(sockfd, buf, strlen(buf));
                    //break;
                    continue;
                }
                if(!strcmp(buf,list))
                {
                    write(sockfd, buf, strlen(buf));
                    continue;
                }
                if(!strcmp(buf,nickname))
                {
                    write(sockfd, buf, strlen(buf));
                    continue;
                }
                if(!strcmp(buf,role))
                {
                    write(sockfd, buf, strlen(buf));
                    continue;
                }
                if(!strcmp(buf,info))
                {
                    write(sockfd, buf, strlen(buf));
                    continue;
                }
                if(!strcmp(buf,kick))
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
                if(strstr(buf,message2) != NULL)
                {
                    write(sockfd, buf, strlen(buf));
                    continue;
                }

                // No caso de uma mensagem de bate-papo geral, não um comando, inclua seu apelido
                // Server에 전송
                sprintf(chatData, "[%s] %s", argv[3], buf);
                write(sockfd, chatData, strlen(chatData));
            }
        }
        /*************************************************************/
    }
}

void
main(int argc,char *argv[])
{
    int sockfd;						// Socket file descriptor
    struct sockaddr_in servaddr;	// Server ip address
    int maxfdp;						// Max file descriptor
    fd_set rset;

    // 파라미터 입력이 정확하지 않은 경우 에러 메시지 출력
    if(argc < 4)
    {
        printf("usage:%s [ip_address] [port_number] [nickname]\n",argv[0]);
        exit(0);
    }

    // IPv4, TCP/IP 소켓 생성
    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
        perror("socket");
        exit(1);
    };

    memset(&servaddr, 0, sizeof(servaddr));			// servaddr 초기화
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);	// 입력받은 IP Address
    servaddr.sin_family=AF_INET;					// IPv4
    servaddr.sin_port=htons(atoi(argv[2]));			// 입력받은 Port number

    // Server에 접속 요청 실패 시 에러 메세지 출력후 종료
    if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        perror("connect");
        exit(0);
    }

    // Server에 Client Nickname 전송
    write(sockfd, argv[3], strlen(argv[3]));
    maxfdp=sockfd + 1;	// maxfdp 증가

    // Chatting function 실행
    chatting(sockfd, maxfdp, rset, argv);

    // Close socket
    close(sockfd);
}