#ifndef CLIMNG
#define CLIMNG

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

#include "gameLogicStructs.h"

#define BUFFSIZE 512

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

void createPipe(int pipeFD[2])
{
    if(pipe(pipeFD) < 0)
    {
        perror("Pipe not created");
        exit(1);
    }
}

void sendTeamResponseByPipe(int pipeWrite, char* response, char* team, char position)
{
    char buffer[10];
    char pos[2]; pos[0] = position + 48; pos[1] = '\0';
    printf("char[2] non è l'errore\n");
    setBuff(buffer, response); strcat(buffer, "-");
    strcat(buffer, team); strcat(buffer, "-"); 
    strcat(buffer, pos);
    printf("La concat di char[2] non è l'errore\n");
    //R-T-P (R->0, T->2, P->4)
    write(pipeWrite, buffer, strlen(buffer));
    printf("Messaggio mandato sulla pipe\n");
}

int recvTeamResponseByPipe(int pipeRead, struct referee Ref, struct player* player)
{
    char buffer[10];
    printf("Prima della read della pipe\n");
    read(pipeRead, buffer, 10);
    printf("Messaggio ricevuto sulla pipe\n");

    if(buffer[0] == '0') { return 0; }

    int pos = buffer[4] - 48;

    if(buffer[2] == 'A') 
        player = & Ref.teamA.members[pos];
    else
    if(buffer[2] == 'B')
        player = & Ref.teamB.members[pos];
    
    printf("Messaggio correttamente decodificato\n");
    
    return 1;
}

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
/* 
int sendSuccessMSG(int socket, char* msg)
{
    char buff[BUFFSIZE] = "(2)";
    strcat(buff, msg);
    return write(socket, buff, strlen(buff));
}
int sendFailureMSG(int socket, char* msg)
{
    char buff[BUFFSIZE] = "(3)";
    strcat(buff, msg);
    return write(socket, buff, strlen(buff));
}
 */
int recMSG(int socket, char* msg)
{

}

#endif