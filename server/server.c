#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

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

void* AcceptNewPlayer(void* socketFD)
{
    int sockFD = *((int*) socketFD);
    if(Ref.gameBool == 0)
    {
        Ref.gameBool = 2;
        char buffer[BUFFSIZE] = "";
        write(sockFD, "(0)Inizio creazione partita\n\n", 30);
        read(sockFD, buffer, BUFFSIZE);

        write(sockFD, "(1)Inserisci nome Squadra A: ", 30);
        read(sockFD, buffer, BUFFSIZE);
        strncpy(Ref.teamA.teamName, buffer, strlen(buffer));
        printf("Il primo team è %s", Ref.teamA.teamName);

        memset(buffer, 0, strlen(buffer));

        write(sockFD, "(1)Inserisci nome Squadra B: ", 30);
        read(sockFD, buffer, BUFFSIZE);
        strncpy(Ref.teamB.teamName, buffer, strlen(buffer));
        printf("Il secondo team è %s", Ref.teamB.teamName);


        printf("La partita è "); printf("%s", Ref.teamA.teamName); printf("-"); printf("%s", Ref.teamB.teamName);

        Ref.gameBool = 1;
    }
    else
    {
        while(Ref.gameBool != 1);

        //aggiungiti alla partita
    }



}

int main()
{
    printf("Welcome to LSOccer Simulator's Server!\n");

    int wsock_fd, new_socket;
    int opt = 1;
    struct sockaddr_in servaddr, cliaddr;
    InitReferee(&Ref);
    
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