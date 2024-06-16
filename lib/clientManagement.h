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
//#include "errorChecking.h"

#define BUFFSIZE 512

int sendMSG(int socket, char* msg);
int sendMSGnoRet(int socket, char* msg);
void sendMSGtoAllClients(struct referee Ref, char* msg);

void setBuff(char* buffer, char* text)
{
    memset(buffer, 0, BUFFSIZE);
    strcpy(buffer, text);
}

void setnBuff(char* buffer, char* text, size_t size)
{
    memset(buffer, 0, size);
    strncpy(buffer, text, size);
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
    char buffer[20];
    char pos[2]; pos[0] = position + 48; pos[1] = '\0';

    setnBuff(buffer, response, 20); strcat(buffer, "-");
    strcat(buffer, team); strcat(buffer, "-"); 
    strcat(buffer, pos);
    //R-T-P (R->0, T->2, P->4)

    write(pipeWrite, buffer, strlen(buffer));
}

int recvTeamResponseByPipe(int pipeRead, struct referee* Ref, struct player** playerPTR, char  msg[25])
{
    char buffer[20];

    read(pipeRead, buffer, 20);

    if(buffer[0] == '0') 
    { 
        sprintf(msg, "Rifiutato da team %c\n", buffer[2]);
        return 0;
    } else if(buffer[2] == 'A') 
    {
        int pos = buffer[4] - 48;
        *playerPTR = &(Ref->teamA.members[pos]);
        setPlayerTeam(*playerPTR, 'A', Ref->teamA.teamName);
        sprintf(msg, "Accettato da team %c\n", buffer[2]);
        return 1;
    } else if(buffer[2] == 'B') 
    {
        int pos = buffer[4] - 48;
        *playerPTR = &(Ref->teamB.members[pos]);
        setPlayerTeam(*playerPTR, 'B', Ref->teamB.teamName);
        sprintf(msg, "Accettato da team %c\n", buffer[2]);
        return 1;
    }
    
}

void sendMSGtoAllClients(struct referee Ref, char* msg)
    {
        int i; char resp[2];
        for(i = 0; i < TEAMSIZE; i++)
        {
            sendMSGnoRet(Ref.teamA.members[i].FD, msg);
            sendMSGnoRet(Ref.teamB.members[i].FD, msg);
        }
    }

int sendMSG(int socket, char* msg)
{
    char buff[BUFFSIZE] = "[0]";
    strcat(buff, msg);
    return write(socket, buff, strlen(buff));
}
int askMSG(int socket, char* msg)
{
    char buff[BUFFSIZE] = "[1]";
    strcat(buff, msg);
    return write(socket, buff, strlen(buff));
}
int sendMSGnoRet(int socket, char* msg)
{
    char buff[BUFFSIZE] = "[2]";
    strcat(buff, msg);
    return write(socket, buff, strlen(buff));
}

enum errType {wrongInput};
char* errors[] =  {"Wrong Input"};


int sendErrorMSG(int socket, enum errType eErr, char* msg)
{
    char buff[BUFFSIZE] = "[3]";
    sprintf(buff, "%s%s", buff, errors[eErr]);
    if (strlen(msg) > 0) {strcat(buff, ": "), strcat(buff, msg);}
    return write(socket, buff, strlen(buff));
}

int sendExitMSG(int socket, char* msg)
{
    char buff[BUFFSIZE] = "[9]";
    strcat(buff, msg);
    return write(socket, buff, strlen(buff));
}

#endif