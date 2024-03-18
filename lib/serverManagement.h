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

//
void writeLog(char* path, char* msg);
void printStatsOfMAtch(struct referee Ref, char EndOfGameMSG[]);
//
void MatchPresentation(int sockFD, char buffer[], struct referee Ref);
void captainConfirmation(int sockFD, char buffer[], struct player* newCapt);
//
void TeamCaptainInitialization(int sockFD, char buffer[], struct referee* Ref, struct player* currPlayer, char team);
void getTeamName(int sockFD, char buffer[], struct referee* Ref, char team);
void NewPlayerInitialization(int sockFD, char buffer[], struct player* newPlayer);
//
void getPlayerName(int sockFD, char buffer[], char name[50]);
int getPlayerNumber(int sockFD, char buffer[], char numS[3]);
//
char TeamRequestChoice(int sockFD, char buffer[], struct referee* Ref);
struct player* TeamMemberRequest(int sockFD, struct referee* Ref, struct player* newPlayer,struct playerQueue* Queue, int pipeRead);
void TeamMemberAcceptance(int sockFD, char buffer[], struct team* Team, char* teamStr , struct playerQueue* Queue, int pipeWrite);
//

void sendMSGtoAllOutputs(struct referee Ref, char msg[])
{
    printf("%s", msg); sendMSGtoAllClients(Ref, msg);
    writeLog(Ref.pathLogServer, msg);
}

//LOG FUNCTIONS

void writeLog(char* path, char* msg)
    {
        FILE* fp = fopen(path, "a");
        fwrite(msg, sizeof(char), strlen(msg), fp);
        fclose(fp);
    }

void printStatsOfMAtch(struct referee Ref, char EndOfGameMSG[])
    {
        writeLog(Ref.pathLogServer, EndOfGameMSG);
        writeLog(Ref.pathLogServer, "\nStatistiche della partita:\n");
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

//OUTPUTS FUNCTIONS
void sendMinuteToAllOutputs(struct referee Ref)
{
    char buffer[15]; int i;
    sprintf(buffer, "Minute: %d\n", Ref.time);
    sendMSGtoAllOutputs(Ref, buffer);
}

void sendConstMinuteToAllOutputs(struct referee Ref, int constMin)
{
    char buffer[15]; int i;
    sprintf(buffer, "Minute: %d\n", constMin);
    printf("%s", buffer); sendMSGtoAllClients(Ref, buffer);
    writeLog(Ref.pathLogServer, buffer);
}

void MatchPresentation(int sockFD, char buffer[], struct referee Ref)
{
    sprintf(buffer, "La partita è %s-%s\n", Ref.teamA.teamName, Ref.teamB.teamName);
    sendMSG(sockFD, buffer); read(sockFD, buffer, BUFFSIZE);
}

void playerConfirmation(int sockFD, char buffer[], struct player* newPlayer)
{
    setBuff(buffer, "");
        
    sprintf(buffer, "Nuovo giocatore: %s con numero %d\n", newPlayer->name, newPlayer->shirtNumber);
    printf("%s", buffer); sendMSG(sockFD, buffer); read(sockFD, buffer, BUFFSIZE); 
}

void captainConfirmation(int sockFD, char buffer[], struct player* newCapt)
{
    setBuff(buffer, "");
        
    sprintf(buffer, "Il capitano della squadra %s è %s con numero %d\n", newCapt->teamName, newCapt->name, newCapt->shirtNumber);
    printf("%s", buffer); sendMSG(sockFD, buffer); read(sockFD, buffer, BUFFSIZE); 
}

void MatchStart(struct referee* Ref)
{
    printf("\nTutto pronto: INIZIA LA PARTITA\n\n");
    sleep(1); sendConstMinuteToAllOutputs(*Ref, 0);
    Ref->gameStatus = gameStarted; Ref->time = 0;
}

void RecoveryTime(char buffer[], struct referee* Ref)
{
    setBuff(buffer, "");
    sprintf(buffer, "Minuti di recupero\n");
    printf("%s", buffer); writeLog(Ref->pathLogServer, buffer);
    sendMSGtoAllClients(*Ref, "Minuti di recupero\n");
}

void MatchFinish(char buffer[], struct referee* Ref)
{
    sprintf(buffer, "\n\nPARTITA FINITA\n%s %d-%d %s\n", Ref->teamA.teamName, Ref->stats.numberGoalA, Ref->stats.numberGoalB, Ref->teamB.teamName);
    printf("%s", buffer); sendMSGtoAllClients(*Ref, buffer);
    printStatsOfMAtch(*Ref, buffer);
}

//PLAYERS SIGN-IN FUNCTIONS

void TeamCaptainInitialization(int sockFD, char buffer[], struct referee* Ref, struct player* currPlayer, char team)
{    

    getTeamName(sockFD, buffer, Ref, team);

    NewPlayerInitialization(sockFD, buffer, currPlayer);

        setBuff(buffer, "");

    if (team == 'A') { setPlayerTeam(currPlayer, 'A', Ref->teamA.teamName);}
    if (team == 'B') { setPlayerTeam(currPlayer, 'B', Ref->teamB.teamName);}

    captainConfirmation(sockFD, buffer, currPlayer);
    
        setBuff(buffer, "");

    while(Ref->gameStatus == oneCaptainNeeded);
    Ref->gameStatus--;

    while(Ref->gameStatus != gameCreated);
}

void getTeamName(int sockFD, char buffer[], struct referee* Ref, char team)
{
        sprintf(buffer, "Sei il capitano della squadra %c\n", team);
        sendMSG(sockFD, buffer); read(sockFD, buffer, BUFFSIZE);
        
            setBuff(buffer, "");

        sprintf(buffer, "Inserisci nome squadra %c: ", team);
        askMSG(sockFD, buffer); read(sockFD, buffer, BUFFSIZE);

        if(team == 'A') {strcpy_noNL(Ref->teamA.teamName, buffer); printf("La squadra %c è %s\n", team, Ref->teamA.teamName);}
        if(team == 'B') {strcpy_noNL(Ref->teamB.teamName, buffer); printf("La squadra %c è %s\n", team, Ref->teamB.teamName);}

          
}

void WaitCaptains(int sockFD, char buffer[], struct referee* Ref)
{
    if(Ref->gameStatus == gameCreation || Ref->gameStatus == waitingOtherCaptain)
    {
        sendMSG(sockFD, "Creazione partita in corso: ATTENDI\n\n");
        read(sockFD, buffer, BUFFSIZE);

        while(Ref->gameStatus != gameCreated);
        sendMSG(sockFD, "Partita creata!\n");
        read(sockFD, buffer, BUFFSIZE);
    }
    else if(Ref->gameStatus == gameCreated)
    {
        sendMSG(sockFD, "Creazione partita completata\n\n");
        read(sockFD, buffer, BUFFSIZE);
    }
}

void WaitFullTeams(int sockFD, char buffer[], struct referee* Ref)
{
    if(Ref->gameStatus < gameStarting)
    {
        sendMSG(sockFD, "Squadre non al completo, attendi inizio partita\n"); read(sockFD, buffer, BUFFSIZE);
        while(Ref->gameStatus != gameStarting);
    }
}

void NewPlayerInitialization(int sockFD, char buffer[], struct player* newPlayer)
{
    setBuff(buffer, "");

        char name[50];
        getPlayerName(sockFD, buffer, name); 

        char sNum[3];
        int num = getPlayerNumber(sockFD, buffer, sNum);

            setBuff(buffer, "");
    
        initPlayer(newPlayer, name, num);
        setPlayerSysVar(newPlayer, sockFD, syscall(__NR_gettid));
}

//PLAYERS DATA FUNCTIONS

void getPlayerName(int sockFD, char buffer[], char name[50])
{
    setBuff(buffer, "");

    askMSG(sockFD, "Inserisci il tuo nome: ");
    read(sockFD, buffer, BUFFSIZE);
    strcpy_noNL(name, buffer);            
}

int getPlayerNumber(int sockFD, char buffer[], char sNum[3])
{
    setBuff(buffer, "");

    askMSG(sockFD, "Inserisci il tuo numero di maglia: ");
    read(sockFD, buffer, BUFFSIZE); int num = atoi(buffer);
    strcpy_noNL(sNum, buffer);
        
    return num;
}

//TEAM-MAKING FUNCTIONS

char ChooseMemberRequest(int sockFD, char buffer[], struct playerQueue* Queue)
{
    sendMSG(sockFD, "Un giocatore ha richiesto di entrare nella tua squadra\n"); read(sockFD, buffer, BUFFSIZE);
    setBuff(buffer, qHead(Queue)->name); strcat(buffer, ", lo vuoi accettare? (Y/N)\n");
    askMSG(sockFD, buffer); read(sockFD, buffer, BUFFSIZE);

    return buffer[0];
}

void newMemberAccepted(int sockFD, char buffer[], struct team* Team, char* teamStr , struct playerQueue* Queue, int pipeWrite)
{
    Team->members[Team->membNum] = * qHead(Queue);
    sendMSG(sockFD, "Giocatore accettato\n"); read(sockFD, buffer, BUFFSIZE);
    sendTeamResponseByPipe(pipeWrite, "1", teamStr, Team->membNum);
    Team->membNum++;
}

void newMemberRejected(int sockFD, char buffer[], struct team* Team, char* teamStr , struct playerQueue* Queue, int pipeWrite)
{
    sendMSG(sockFD, "Giocatore rifiutato\n"); read(sockFD, buffer, BUFFSIZE);
    sendTeamResponseByPipe(pipeWrite, "0", teamStr, Team->membNum);
}

void TeamMemberAcceptance(int sockFD, char buffer[], struct team* Team, char* teamStr , struct playerQueue* Queue, int pipeWrite)
{
    while(Team->membNum < TEAMSIZE)
    {
        while(qIsEmpty(Queue));

        switch (ChooseMemberRequest(sockFD, buffer, Queue))
        {
        case 'Y': newMemberAccepted(sockFD, buffer, Team, teamStr , Queue, pipeWrite); 
                  qNext(Queue); 
            break;
        case 'N': newMemberRejected(sockFD, buffer, Team, teamStr , Queue, pipeWrite); 
                  qNext(Queue); 
            break;
        default : sendErrorMSG();
            break;
        }             
    }
}

char TeamRequestChoice(int sockFD, char buffer[], struct referee* Ref)
    {
        char ansBuff[BUFFSIZE];

        sprintf(buffer, "1-> %s; 2-> %s\n", Ref->teamA.teamName, Ref->teamB.teamName);
        sendMSG(sockFD, buffer); read(sockFD, buffer, BUFFSIZE);

        setBuff(buffer, "");

        askMSG(sockFD, "Decidi di che squadra vorresti far parte: "); 
        read(sockFD, ansBuff, BUFFSIZE);
        char teamChoice = atoi(ansBuff);

        return teamChoice;
    }

struct player* TeamMemberRequest(int sockFD, struct referee* Ref, struct player* newPlayer,struct playerQueue* Queue, int pipeRead)
    {
            char buffer[BUFFSIZE];
            struct player* retPlayerPTR = 0;

            
                qInsert(Queue, newPlayer);
                char msg [25]; recvTeamResponseByPipe(pipeRead, Ref, &retPlayerPTR, msg);
                sendMSG(sockFD, msg); read(sockFD, buffer, BUFFSIZE);


            setBuff(buffer, "");

            return retPlayerPTR;
    }

#endif
