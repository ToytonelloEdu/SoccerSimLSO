#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>

#include <time.h>
#include <pthread.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <asm-generic/socket.h>

#include "../lib/gameLogicStructs.h"
#include "../lib/gameLogicFuncts.h"

#define BUFFSIZE 512

struct referee Ref;

int sendMSG(int socket, char* msg)
{
    char buff[BUFFSIZE] = "(0)";
    strcat(buff, msg);
    return write(socket, buff, strlen(buff));
}
int askMSG(int socket, char* msg)
{
    char buff[BUFFSIZE] = "(1)";
    strcat(buff, msg);
    return write(socket, buff, strlen(buff));
}
int recMSG(int socket, char* msg)
{

}
void setBuff(char* buffer, char* text)
{
    memset(buffer, 0, BUFFSIZE);
    strcpy(buffer, text);
}

void strcpy_noNL(char* dest, char* source)
{
    strncpy(dest, source, strlen(source));
    dest[strcspn(dest, "\n")] = 0;
}

void* AcceptNewPlayer(void* socketFD)
{
    int sockFD = *((int*) socketFD);
    if(Ref.gameStatus == 0)
    {
        Ref.gameStatus = 2;
        char buffer[BUFFSIZE] = "";
        //char msgBuff[BUFFSIZE] = "";
        sendMSG(sockFD, "Inizio creazione partita\n\n");
        read(sockFD, buffer, BUFFSIZE);

        askMSG(sockFD, "Inserisci nome Squadra A: ");
        read(sockFD, buffer, BUFFSIZE);
        strncpy(Ref.teamA.teamName, buffer, strlen(buffer));
        printf("Il primo team è %s", Ref.teamA.teamName);

        setBuff(buffer, "");

        askMSG(sockFD, "Inserisci nome Squadra B: ");
        read(sockFD, buffer, BUFFSIZE);
        strncpy(Ref.teamB.teamName, buffer, strlen(buffer));
        printf("Il secondo team è %s", Ref.teamB.teamName);

        Ref.teamA.teamName[strcspn(Ref.teamA.teamName, "\n")] = 0;
        setBuff(buffer, "La partita è ");
        strcat(buffer, Ref.teamA.teamName);
        strcat(buffer, "-");
        strcat(buffer, Ref.teamB.teamName);
        Ref.teamB.teamName[strcspn(Ref.teamB.teamName, "\n")] = 0;
        
        printf("%s\n", buffer);
        sendMSG(sockFD, buffer);
        read(sockFD, buffer, BUFFSIZE);


        setBuff(buffer, "");

        sendMSG(sockFD, "Sei il capitano della squadra A\n");
        read(sockFD, buffer, BUFFSIZE);

        askMSG(sockFD, "Inserisci il tuo nome: ");
        read(sockFD, buffer, BUFFSIZE);
        char name[50]; 
        strcpy_noNL(name, buffer);

        setBuff(buffer, "");

        askMSG(sockFD, "Inserisci il tuo numero di maglia: ");
        read(sockFD, buffer, BUFFSIZE); 
        int num = atoi(buffer);

        setBuff(buffer, "");

        initPlayer(Ref.teamA.captain, name, num, Ref.teamA.teamName);
        Ref.teamA.captain->playerFD = sockFD;
        Ref.teamA.captain->playerTID = syscall(__NR_gettid);

        printPlayer(Ref.teamA.captain);


        





        Ref.gameStatus = 1;
    }
    else
    {
        while(Ref.gameStatus != 1);

        
    }



}

int main()
{
    printf("Welcome to LSOccer Simulator's Server!\n");

    int wsock_fd, new_socket;
    int opt = 1;
    struct sockaddr_in servaddr, cliaddr;
    InitReferee(&Ref);
    initTeam(&(Ref.teamA));
    initTeam(&(Ref.teamB));
    
    if((wsock_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }
    if(setsockopt(wsock_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(12345);

    if(bind(wsock_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if(listen(wsock_fd, 10) < 0)
    {
        perror("listen error");
        exit(EXIT_FAILURE);
    }

    int addrlen = sizeof(cliaddr);

    while((new_socket = accept(wsock_fd, (struct sockaddr*)&cliaddr, (socklen_t*)&addrlen)) > -1)
    {
        printf("New player connected\n");
        pthread_create(&Ref.lastThread, NULL, AcceptNewPlayer, (void*) &new_socket);
        pthread_detach(Ref.lastThread);
    }

    close (wsock_fd);

    return 0;
}