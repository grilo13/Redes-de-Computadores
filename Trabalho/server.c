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

// Comandos usados
char ERROR[]= "Erro.\n";
char quit[]="QUIT\n";
char whos[]="WHOS\n";
char message[]="MSSG";
char nickname[]="NICK";
char kick[]="KICK";
char role[]="OPER";
char user_info[]="INFO\n";
char exists[]="EXISTS\n";
char register1[]="REGS";
char list_regs[]="LISTR\n";
char authenticate[]="PASS";
char exit_chat[]="EXIT\n";

//comando do admin (usado para a criação inicial do mesmo, e onde se atribui os valores corretos ao mesmo)
char admin[]="ADMI";

// struct que representa os clientes 
struct List_c{
    int socket_num;          //socket id
    int first_command;       // usado para ver se o primeiro input que o cliente faz é o NICK, usado posteriormente
    char nick[NICKNAME];    // nickname associado ao cliente
    char ip[40];            // ip associado ao cliente
    int port;               // porta do server usada
    char role[ROLESIZE];    //categoria associada ao cliente = user (ou operador)                
}list_c[MAX_CLIENT];

//struct que representa os clientes registados
struct Register_c{    
    int socket_num;             //socket id
    char nick[CHATDATA];        // nickname associado ao cliente (igual ao normal)
    char password[CHATDATA];  //password associado ao cliente (feito no registo)
    char role[ROLESIZE];      //categoria associada ao cliente
    char auth[ROLESIZE];        //para saber se o cliente esta autenticado ou não
}register_c[MAX_CLIENT];

//Função que é apenas usada uma vez (para atribuir ao ADMIN os valores necessários iniciais)
void give_admin_role(char* chatData, int i){
    int j;
    char* token;
    char buf1[MAXLINE];
    int contador = 0;
    char* aux2[20];
    char* aux3[20];
    char* aux4 = "autenticado";
    char* end = " ";

    token=strtok(chatData, end);

    char* aux = "operador";

    memset(buf1,0,sizeof(buf1));

   if(strcmp(token, admin)==0){
        token = strtok(NULL, end);
        strcpy(aux2,token);
        token = strtok(NULL, end);
        token[strcspn(token, "\n")] = 0;
        strcpy(aux3,token);
   }

   strcpy(list_c[i].role, aux);
   strcpy(register_c[i].nick,aux2);
   strcpy(register_c[i].password,aux3);
   strcpy(register_c[i].role,aux);
   strcpy(register_c[i].auth,aux4);

   sprintf(buf1,"Olá ADMIN. INFO: %s, %s, %s, %s.\n", register_c[i].nick, register_c[i].password, register_c[i].role, register_c[i].auth);
   write(list_c[i].socket_num,buf1,strlen(buf1));
}


//Função que insere um novo cliente na lista de clientes disponivel
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

//Função que retira um cliente da lista de clientes (usada na exit_func para quando o cliente quer sair do canal)
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

//Função que retira o utilizador do canal (o mesmo pede para sair)
void exit_func(int i){
    int j;
    char* token=NULL;
    char buf1[MAXLINE];

    memset(buf1,0,sizeof(buf1));
    printf("%s left the channel from %s.\n",list_c[i].nick, list_c[i].ip);
    for(j=0; j<MAX_CLIENT;j++)
        if(j!=i && list_c[j].socket_num!=INVALID_SOCK){
            sprintf(buf1,"%s left the channel.\n",list_c[i].nick);
            write(list_c[j].socket_num,buf1,strlen(buf1));
        }
}

//Função auxiliar que atribui o nickname de um cliente (usada na função nick_func)
void set_nick(char* c_nick, int i){
    strcpy(list_c[i].nick,c_nick);
}

//Função NICK - atribui o nickname ao cliente.
// se for novo, atribui o nome
// se for igual a outro existente, avisa
// se substituir avisa que utilizador substitui
// se nick tiver mais caracteres que o permite, não cria e avisa o mesmo
// se não for introduzido nenhum nome, avisa o mesmo também
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
                puts("RPLY 002 - Erro: Falta introdução do nome.\n\n");
                sprintf(buf1,"RPLY 002 - Erro: Falta introdução do nome.\n\n");
                write(list_c[i].socket_num,buf1,strlen(buf1));
            }
        }else{
            if(list_c[i].socket_num!=INVALID_SOCK){
                if(strlen(token) > NICKNAME){
                    puts("RPLY 003 - Erro: Nome pedido não válido. (Excede nº máximo de carateres permitido ou utiliza carateres inválidos\n\n");
                    sprintf(buf1,"RPLY 003 - Erro: Nome pedido não válido. (Excede nº máximo de carateres permitido ou utiliza carateres inválidos\n\n");
                    write(list_c[i].socket_num,buf1,strlen(buf1));
                }else if(strlen(token) < NICKNAME){
                    for(int j = 0; j < MAX_CLIENT; j++){
                        if(strcmp(list_c[j].nick,token)==0){
                            puts("RPLY 004 - Erro: nome já em uso (num outro utilizador registado ou em uso por um utilizador não registado, e o comando não tem qualquer efeito\n\n");
                            sprintf(buf1,"RPLY 004 - Erro: nome já em uso (num outro utilizador registado ou em uso por um utilizador não registado, e o comando não tem qualquer efeito\n\n");
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
                            puts("RPLY 001 - Nome atribuído com sucesso\n\n");
                            sprintf(buf1,"RPLY 001 - Nome atribuído com sucesso\n\n");
                            write(list_c[i].socket_num,buf1,strlen(buf1));
                        
                        }else{
                            set_nick(token, i);
                            puts("RPLY 001 - Nome atribuído com sucesso\n\n");
                            sprintf(buf1,"RPLY 001 - Nome atribuído com sucesso\n\n");
                            write(list_c[i].socket_num,buf1,strlen(buf1));
                            printf("%s has been connected from %s\n\n", list_c[i].nick, list_c[i].ip);
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
                    sprintf(buf1,"MSSG server :> <%s> mudou o seu nome para <%s>\r\n\n", old_nick, list_c[i].nick);
                    write(list_c[j].socket_num,buf1,strlen(buf1));
                }
            }
        }
    }
}

//Função PASS - autentica um cliente já registado
// se utilizador não existir, é dado um erro
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

// Função que mostra ao client a sua informação (o seu nick, a sua categoria no canal, e o seu ip)
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

//Função auxiliar que retorna -1 se o cliente procurado não existir, ou id=j, se cliente for encontrado no indice j
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

// Função que remove o cliente da sua função de operador
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

//Função que atribui a um client a categoria de operador
// só pode ser dada por um cliente que já seja operador, e que esteja autenticado
// o cliente a que vai ser atribuido já tem que estar registado também
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

// Funão que atribui ao cliente a registar-se, o nickname, a password, a role e o valor de autenticação iniciais
// auth inicial = não_autenticado
void give_password(int i, char* password){
    int j,cnt=0;
    char buf1[MAXLINE];

    char* aux3 = "não_autenticado";
    int id = 0;

    memset(buf1,0,sizeof(buf1));

    strcpy(register_c[i].password, password);
    strcpy(register_c[i].nick, list_c[i].nick);
    strcpy(register_c[i].role, list_c[i].role);
    strcpy(register_c[i].auth, aux3);

    sprintf(buf1,"RPLY 701 – Utilizador foi registado com sucesso.\n");
    write(list_c[i].socket_num,buf1,strlen(buf1));
    printf("RPLY 701 – Utilizador foi registado com sucesso.\n");

}

//Função que mostra a lista de pessoas registadas no canal (e a sua informação, NICK, PASSWORD, e AUTH)
// usada como apoio na realização do trabalho
void list_registered(int i){
    int j,cnt=0;
    char buf1[MAXLINE];

    memset(buf1,0,sizeof(buf1));

    for(j=0; j<MAX_CLIENT;j++){
        int tamanho = strlen(register_c[j].nick);
        if(tamanho > 0){
            cnt++;
            sprintf(buf1,"[Utilizador registado : %s, %s, %s]\n",register_c[j].nick, register_c[j].password, register_c[j].auth);
            write(list_c[i].socket_num,buf1,strlen(buf1));
        }
    }

    sprintf(buf1,"[Nº de utilizadores registados : %d]\n",cnt);
    write(list_c[i].socket_num,buf1,strlen(buf1));        
}

// Função que regista o utilizador pretendido por parte do cliente.
// se utilizador não existir, cliente é avisado que o mesmo não existe
// cliente tem de ser operador e estar autenticado
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
    char* autenticado = "autenticado";
    //strcpy(aux,"OPERADOR");

    memset(buf1,0,sizeof(buf1));

    if(strcmp(list_c[i].role,aux) == 0){  //se utilizador for operador

        if(strcmp(register_c[i].auth,autenticado) == 0){
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
            sprintf(buf1,"Erro. Cliente não está autenticado (ou registado).\n");
            write(list_c[i].socket_num,buf1,strlen(buf1));
            puts("RPLY 702 – Erro. Ação não autorizada, utilizador cliente não é um operador.\n");
        }

    } else {
        sprintf(buf1,"RPLY 702 – Erro. Ação não autorizada, utilizador cliente não é um operador.\n");
        write(list_c[i].socket_num,buf1,strlen(buf1));
        puts("RPLY 702 – Erro. Ação não autorizada, utilizador cliente não é um operador.\n");
    }
}

// Função que remove um utilizador da lista de utilizadores registados por parte de um cliente
// o mesmo tem que ser operador
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
// Função auxiliar para remover os atributos de registo ao utilizador que vai ser removido
void remove_register(int i,char* user){
    int j;
    char buf1[MAXLINE];

    char* aux="";
    char* role="operador";
    char* aux4="user";
    char* aux5 = "não_autenticado";

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
                strcpy(register_c[j].auth, aux5);
                strcpy(list_c[j].role, aux4);

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
        } else if(decisao == 2){
            for(j=0; j<MAX_CLIENT;j++){
                if(j!= i && list_c[j].socket_num!=INVALID_SOCK){
                    sprintf(buf1,"MSSG server :> <%s> foi expulso.\n", user);
                    write(list_c[j].socket_num,buf1,strlen(buf1));
                }
            }
        }

    } else {
        sprintf(buf1,"\nRPLY 602 – Erro. Ação não autorizada, utilizador %s não é um operador.", list_c[i].nick);
        write(list_c[i].socket_num,buf1,strlen(buf1));
    }
}

// Função que mostr a lista de utilizadores atualmente no canal (e a sua informação, NICK, ROLE e IP)
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

// Função que representa a resposta do servidor às mensagens que podem ser introduzidas pelo cliente
int message_func(char* chatData,int i){

    char* token;
    char buf1[MAXLINE];
    char* end = " ";

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
                return 1;
            }
        }else{
            if(list_c[i].socket_num!=INVALID_SOCK){
                if(strlen(token) > CHATDATA){
                    puts("RPLY 103 - Erro. Mensagem demasiado longa.\n");
                    sprintf(buf1,"RPLY 103 - Erro. Mensagem demasiado longa.\n");
                    write(list_c[i].socket_num,buf1,strlen(buf1));
                    return 1;
                }else if(strlen(token) < CHATDATA){
                    puts("RPLY 101 - mensagem enviada com sucesso.\n");
                    sprintf(buf1,"RPLY 101 - mensagem enviada com sucesso.\n");
                    write(list_c[i].socket_num,buf1,strlen(buf1));
                    return 0;
                }
            }
        }
    }else{ 
        if(list_c[i].socket_num!=INVALID_SOCK){
            puts("RPLY 102 - Erro. Não há texto na mensagem.\n"); 
            sprintf(buf1,"RPLY 102 - Erro. Não há texto na mensagem.\n");
            write(list_c[i].socket_num,buf1,strlen(buf1)); 
            return 1;
        }
    }
}

// Função auxiliar que remove a primeira string (comando), sobrando então apenas a mensagem enviada pelo cliente
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

        if(found == 1)
        {
            for(j=i; j<=len-removeLen; j++)
            {
                str[j] = str[j + removeLen];
            }

            break;
        }
    }
}

// Função main, onde são utilizadas todas aquelas mostradas em cima //
void main(int argc, char *argv[]) {
    int newSockfd,sockfd;                  
    struct sockaddr_in servaddr;
    int maxfd=0;                            
    int i,j,n;
    fd_set rset;

    int index;

    char* end = " ";
    char* token=NULL;
    char buf1[MAXLINE];
    char buf2[MAXLINE];
    char chatData[CHATDATA];
    char new_chatData[CHATDATA];

    // ./server [port_number] inicialização do servidor, se nenhum mensagem for introduzida é enviada uma error message
    if(argc<2){
        printf("usage: %s port_number\n",argv[0]);
        exit(-1);
    }

    // criação da socket
    if ((sockfd=socket(AF_INET,SOCK_STREAM,0)) <= 0){
        perror("socket");
        exit(1);
    }

    // Informações necessárias para o socket
    memset(&servaddr,0,sizeof(servaddr));       //guarda as informações dos clientes
    servaddr.sin_addr.s_addr=INADDR_ANY;
    servaddr.sin_family=AF_INET;                 //endereço IPV4
    servaddr.sin_port=htons(atoi(argv[1]));      //atribuição da porta

    // Faz bind à porta
    if(bind(sockfd,(struct sockaddr *) &servaddr,sizeof(servaddr)) < 0){
        printf("Error : Kill process.\n");
        exit(1);
    }

    // confirmação da conexão do cliente com o servidor
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

    char *message2 = "                                   MENU                                         \n\n NICK <nickname> --> ATRIBUI/ALTERA O NICK DO UTILIZADOR \n\n MSSG <texto> --> NECESSÁRIA PARA ENVIAR MENSAGENS AOS OUTROS UTILIZADORES \n\n PASS <password> --> DEFINE UMA PASSWORD PARA NO REGISTO DO UTILIZADOR \n\n JOIN <canal> --> MUDA O CANAL ATIVO PARA <CANAL> \n\n LIST --> LISTA OS CANAIS EXISTENTES \n\n WHOS --> LISTA E MOSTRA A INFORMAÇÃO DOS UTILIZADORES NO CANAL ATIVO \n\n";

    char *message3 = "Session started! \n set your nickname with command NICK first! \n\n";


    for ( ; ; )
    {
        maxfd=sockfd;

        FD_ZERO(&rset);              // Limpa o conjunto de select
        FD_SET(sockfd, &rset);       //Coloca o sockfd no conjunto
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

        int addrlen = sizeof(servaddr);

        if(FD_ISSET(sockfd, &rset)){
            
            
            // Aguardar até que a conexão com o cliente seja estabelecida
            // Quando aceite, um novo socket denominado newSockfd é criado. Envia e recebe os dados usando o NewSockFd
            if((newSockfd=accept(sockfd, (struct sockaddr *)&servaddr , (socklen_t *) &addrlen)) > 0) {

                memset(buf2, 0, sizeof(buf2));

                inet_ntop(AF_INET, &servaddr.sin_addr, buf2, sizeof(buf2));
             
                index = pushClient(newSockfd, buf2, save_port); //insere um novo cliente usando a função criada anteriormente PushCliente


                //mensagens de boas vindas dadas ao cliente
                message3 = "\n\nBEM-VINDO \n\n DEFINE O TEU NICKNAME PARA COMEÇAR --> NICK <nickname>  \n\n";

                message2 = "\n\n                                   MENU                                         \n\n NICK <nickname> --> ATRIBUI/ALTERA O NICK DO UTILIZADOR \n\n MSSG <texto> --> NECESSÁRIA PARA ENVIAR MENSAGENS AOS OUTROS UTILIZADORES \n\n PASS <password> --> DEFINE UMA PASSWORD PARA NO REGISTO DO UTILIZADOR \n\n JOIN <canal> --> MUDA O CANAL ATIVO PARA <CANAL> (Indisponínel) \n\n LIST --> LISTA OS CANAIS EXISTENTES (Indisponínel) \n\n WHOS --> LISTA E MOSTRA A INFORMAÇÃO DOS UTILIZADORES NO CANAL ATIVO \n\n INFO --> PARA SABERES INFORMAÇÃO ACERCA DA TUA CONTA \n\n KICK <nome> --> SENDO OPERADOR, PARA EXPULAR O UTILIZADOR COM O <nome> \n\n REGS <nome> <password> --> PARA REGISTAR UM UTILIZADOR \n\n OPER <nome> --> PROMOVE O UTILIZADOR À CATEGORIA DE OPERADOR \n\n QUIT --> DESISTE DE SER OPERADOR \n\n EXIT --> PARA O UTILIZADOR SAIR DO CANAL";


                //envia às messagens ao cliente, se por algum motivo não forem enviadas é mostrado um erro
                if( send(newSockfd, message2, strlen(message2), 0) != strlen(message2) ) {
                    
                    perror("send failed");
                }

                if( send(newSockfd, message3, strlen(message3), 0) != strlen(message3) ) {
                    
                    perror("send failed");
                }

                //mostra apenas na tela do servidor que foi inserido um novo utilizador
                puts("...MENSAGENS DE BOAS-VINDAS ENVIADAS AO UTILIZADOR\n");
                
                if(index < 0){
                    write(newSockfd,ERROR,strlen(ERROR));
                    close(newSockfd);
                }
            }
        }



        //comandos que podem ser introduzidos por todos os utilizadores já existentes
        // vai ver qual é o indice do mesmo, e é esse que vai ser utilizador nas funções para ver permissões, etc...
        for(i=0; i<MAX_CLIENT;i++){
            if((list_c[i].socket_num != INVALID_SOCK) && FD_ISSET(list_c[i].socket_num,&rset)){
                memset(chatData, 0, sizeof(chatData));
                if((n=read(list_c[i].socket_num,chatData, sizeof (chatData)))>0){

                    //COMANDOS QUE PODEM SER INSERIDOS, E QUE RETORNARÃO ALGO

                    //nickname = adiciona o nickname ao cliente, NICK
                    if(strstr(chatData, nickname) != NULL){
                        nick_func(chatData, i);
                        list_c[i].first_command = 1; 
                        continue;
                    }

                    //vê se o comando nick é ou não, o primeiro comando inserido (tem que ser obrigatoriamente)
                    if(list_c[i].first_command != 1){
                        memset(buf2, 0, sizeof(buf2));
                        sprintf(buf2,"NICK COMMMAND SHOULD BE FIRST \n");
                        write(list_c[i].socket_num,buf2,strlen(buf2));
                        continue;

                    }else{
                        
                        //quit = retira a categoria "operador" de um cliente, = QUIT
                        if(!strcmp(chatData, quit)){   
                            quit_func(i);
                            continue;
                        }

                        //whos = lista e mostra a informação dos utilizadores ativos no canal = WHOS
                        if(!strcmp(chatData,whos)){ 
                            whos_func(i);
                            continue;
                        }

                        //message = troca de mensagens entre os utilizadores, MSSG
                        if(strstr(chatData, message) != NULL){
                            memset(buf1,0, sizeof(buf1));
                            strcpy(new_chatData, chatData);
                            removeFirst(chatData, message);
                            if(message_func(new_chatData, i)==0){;
                                for(j=0; j<MAX_CLIENT;j++){   
                                    if(list_c[j].socket_num !=INVALID_SOCK){
                                        if(list_c[j].socket_num != list_c[i].socket_num){
                                            sprintf(buf1,"\nMSSG < %s/%s > : >%s \n", list_c[i].nick, list_c[i].role, chatData);
                                            write(list_c[j].socket_num,buf1,strlen(buf1));
                                        }
                                    }
                                }
                            }
                        continue;
                        }

                        //register1 =  regista um utilizador (atribui-lhe nick e password, se ele existir, e mediante condições pedidas) = REGS
                        if(strstr(chatData, register1) != NULL){
                            user_register(chatData,i);
                            continue;
                        }

                        //list_register = lista e mostra a informação dos utilizadores registados no canal = LISTR
                        if(!strcmp(chatData,list_regs)){
                            list_registered(i);
                            continue;
                        }

                        //authentica = autentica um cliente, mediante as condições pedidas, = PASS
                        if(strstr(chatData, authenticate) != NULL){
                            authenticate1(chatData,i);
                            continue;
                        }

                        //kick = remove dos utilizadores registados um cliente (se o mesmo lá estiver) = KICK
                        if(strstr(chatData, kick) != NULL){
                            kick_user(chatData, i);
                            continue;
                        }

                        //user_info = mostra a informação do cliente (nick, categoria e ip) = INFO
                        if(!strcmp(chatData,user_info)){
                            show_client_info(i);
                            continue;
                        }

                        //role = permite atribuir a categoria OPERADOR a um utilizador registado (apenas um operador o pode fazer) = OPER
                        if(strstr(chatData, role) != NULL){
                            give_role(chatData, i);
                            continue;
                        }

                        //admin = apenas usado pelo admin, com o intuito de atribuir o nick, pass e autenticar automaticamente o admin = ADMI
                        if(strstr(chatData, admin) != NULL){
                            give_admin_role(chatData, i);
                            continue;
                        }

                        //exit_chat = utilizador sai do canal onde está= EXIT
                        if(!strcmp(chatData,exit_chat)){ 
                            exit_func(i);
                            popClient(list_c[i].socket_num);
                            continue;
                        }
                    }
                }
            }
        }
    }
}