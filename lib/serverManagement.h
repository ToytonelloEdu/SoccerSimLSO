#ifndef SRVMNG
#define SRVMNG

#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include "gameLogicStructs.h"
#include "clientManagement.h"
#define QSIZE 15

struct playerQueue
{
    short added;
    short viewed;
    struct player players[QSIZE];
};

int qIsEmpty(struct playerQueue* q)
{
    return q->added == q->viewed;
}

struct player* qHead(struct playerQueue* q)
{
    if(! qIsEmpty(q))
        return & q->players[q->viewed];
    else return NULL;
}

void qNext(struct playerQueue* q)
{
    q->viewed = (q->viewed+1)%QSIZE;
}

void qInsert(struct playerQueue* q, struct player* newPlayer)
{
    q->players[q->added] = *newPlayer;
    q->added = (q->added+1)%QSIZE;
}

void writeLog(char* path, char* msg);
void printStatsOfMAtch(struct referee Ref);
void TeamCaptainInitialization(int sockFD, char buffer[], struct referee* Ref, struct player* currPlayer, char team);
void TeamMemberAcceptance(int sockFD, char buffer[], struct team* Team, char* teamStr , struct playerQueue* Queue, int pipeWrite);

void writeLog(char* path, char* msg)
    {
        FILE* fp = fopen(path, "a");
        fwrite(msg, sizeof(char), strlen(msg), fp);
        fclose(fp);
    }

void printStatsOfMAtch(struct referee Ref)
    {
        char msg[BUFFSIZE] = "";
        int totalGoal = Ref.stats.numberGoalA + Ref.stats.numberGoalB;
        
        sprintf(msg, "GOAL segnati nel match: %d\n", totalGoal);
        writeLog(Ref.pathLogServer, msg);
        setBuff(msg, "");
        
        sprintf(msg, "SHOT falliti nel match: %d\n", Ref.stats.shotFailed);
        writeLog(Ref.pathLogServer, msg);
        setBuff(msg, "");

        sprintf(msg, "DRIBBLING effettuati nel match: %d\n", Ref.stats.numberDribbling);
        writeLog(Ref.pathLogServer, msg);
        setBuff(msg, "");
    }

    void TeamCaptainInitialization(int sockFD, char buffer[], struct referee* Ref, struct player* currPlayer, char team)
    {    

        if (team == 'A')
        {
            setBuff(buffer, "Sei il capitano della squadra A\n");
            sendMSG(sockFD, buffer); read(sockFD, buffer, BUFFSIZE);
            
                setBuff(buffer, "");

            askMSG(sockFD, "Inserisci nome Squadra A: "); 
            read(sockFD, buffer, BUFFSIZE); strcpy_noNL(Ref->teamA.teamName, buffer);
            printf("Il primo team è %s\n", Ref->teamA.teamName);     
        }

        if (team == 'B')
        {
            setBuff(buffer, "Sei il capitano della squadra B\n");
            sendMSG(sockFD, buffer); read(sockFD, buffer, BUFFSIZE);
            
                setBuff(buffer, "");

            askMSG(sockFD, "Inserisci nome Squadra B: ");
            read(sockFD, buffer, BUFFSIZE); strcpy_noNL(Ref->teamB.teamName, buffer);
            printf("Il secondo team è %s\n", Ref->teamB.teamName);
        }

            setBuff(buffer, "");

        askMSG(sockFD, "Inserisci il tuo nome: ");
        read(sockFD, buffer, BUFFSIZE);
        char name[50]; strcpy_noNL(name, buffer);

            setBuff(buffer, "");

        askMSG(sockFD, "Inserisci il tuo numero di maglia: ");
        read(sockFD, buffer, BUFFSIZE); int num = atoi(buffer);
        char sNum[3]; strcpy_noNL(sNum, buffer); 

            setBuff(buffer, "");


        if (team == 'A') { initPlayer(currPlayer, name, num); setPlayerTeam(currPlayer, 'A', Ref->teamA.teamName);}
        if (team == 'B') { initPlayer(currPlayer, name, num); setPlayerTeam(currPlayer, 'B', Ref->teamB.teamName);}

        currPlayer->playerFD = sockFD;
        currPlayer->playerTID = syscall(__NR_gettid);


        setBuff(buffer, "Il capitano della squadra "); strcat(buffer, currPlayer->teamName);
        strcat(buffer, " è "); strcat(buffer, currPlayer->name);
        strcat(buffer, " con numero "); strcat(buffer, sNum); strcat(buffer, "\n");
        
        sendMSG(sockFD, buffer); read(sockFD, buffer, BUFFSIZE);
        printf("%s", buffer);

            setBuff(buffer, "");

        

        while(Ref->gameStatus == oneCaptainNeeded);
        Ref->gameStatus--;

        while(Ref->gameStatus != gameCreated);
    }

    void TeamMemberAcceptance(int sockFD, char buffer[], struct team* Team, char* teamStr , struct playerQueue* Queue, int pipeWrite)
    {
        //short* added = & Queue->added;
        //short* viewed = & Queue->viewed;

        while(Team->membNum < TEAMSIZE)
        {
            while(qIsEmpty(Queue));

            sendMSG(sockFD, "Un giocatore ha richiesto di entrare nella tua squadra\n"); read(sockFD, buffer, BUFFSIZE);
            setBuff(buffer, qHead(Queue)->name); strcat(buffer, ", lo vuoi accettare? (Y/N)\n");
            askMSG(sockFD, buffer); read(sockFD, buffer, BUFFSIZE);
            char answer = buffer[0];

            if(answer == 'Y')
            {
                Team->members[Team->membNum] = * qHead(Queue);
                sendMSG(sockFD, "Giocatore accettato\n"); read(sockFD, buffer, BUFFSIZE);
                sendTeamResponseByPipe(pipeWrite, "1", teamStr, Team->membNum);
                Team->membNum++;
            }
            else if(answer == 'N')
            {
                sendMSG(sockFD, "Giocatore rifiutato\n"); read(sockFD, buffer, BUFFSIZE);
                sendTeamResponseByPipe(pipeWrite, "0", teamStr, Team->membNum); 
            }

            qNext(Queue);                
        }
    }

    char TeamRequestChoice(int sockFD, char* buffer, struct referee* Ref)
    {
        char ansBuff[BUFFSIZE];
        setBuff(buffer, "1-> "); strcat(buffer, Ref->teamA.teamName);
        strcat(buffer, "; 2-> "); strcat(buffer, Ref->teamB.teamName);
        strcat(buffer, "\n"); sendMSG(sockFD, buffer); read(sockFD, buffer, BUFFSIZE);

        setBuff(buffer, "");

        
        askMSG(sockFD, "Decidi di che squadra far parte: "); 
        read(sockFD, ansBuff, BUFFSIZE);
        char teamChoice = atoi(ansBuff);

        return teamChoice;
    }

    struct player* TeamMemberRequest(int sockFD, struct referee* Ref, struct player* newPlayer,struct playerQueue* Queue, int pipeRead)
    {
            char buffer[BUFFSIZE];
            struct player* retPlayerPTR = 0;

            
                qInsert(Queue, newPlayer);
                char msg [25] = recvTeamResponseByPipe(pipeRead, &Ref, retPlayerPTR);
                sendMSG(sockFD, msg); read(sockFD, buffer, BUFFSIZE);


            setBuff(buffer, "");
    }

#endif
