#ifndef SRVMNG
#define SRVMNG

#include <time.h>
#include <stdio.h>
#include <fcntl.h>

#include "gameLogicStructs.h"
#include "clientManagement.h"
#include "errorChecking.h"

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
void getPlayerName(int sockFD, char buffer[], char name[NAMESIZE]);
int getPlayerNumber(int sockFD, char buffer[], char numS[5]);
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
    Ref->gameStatus = gameFinished;
    printf("%s", buffer); sendMSGtoAllClients(*Ref, buffer); printStatsOfMAtch(*Ref, buffer);
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

void askTeamName(int sockFD, char buffer[], char name[], char team)
{
    sprintf(buffer, "Inserisci nome squadra %c: ", team);
    askMSG(sockFD, buffer); read(sockFD, buffer, BUFFSIZE);
    strcpy_noNL(name, buffer);
}

void getTeamName(int sockFD, char buffer[], struct referee* Ref, char team)
{
        sprintf(buffer, "Sei il capitano della squadra %c\n", team);
        sendMSG(sockFD, buffer); read(sockFD, buffer, BUFFSIZE);
        
            setBuff(buffer, "");

        char name[NAMESIZE];
        askTeamName(sockFD, buffer, name, team);

            while(ValidName(name) == 0)
            {
                sendErrorMSG(sockFD, wrongInput, "team name not valid, must be a word with Capital initial and no numbers\n");
                read(sockFD, buffer, BUFFSIZE); setBuff(buffer, ""); 
                askTeamName(sockFD, buffer, name, team);
            } 
       
        

        if(team == 'A') {strcpy_noNL(Ref->teamA.teamName, name); printf("La squadra %c è %s\n", team, Ref->teamA.teamName);}
        if(team == 'B') {strcpy_noNL(Ref->teamB.teamName, name); printf("La squadra %c è %s\n", team, Ref->teamB.teamName);}

          
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

        char name[NAMESIZE];
        getPlayerName(sockFD, buffer, name); 

        char sNum[5];
        int num = getPlayerNumber(sockFD, buffer, sNum);

            setBuff(buffer, "");
    
        initPlayer(newPlayer, name, num);
        setPlayerSysVar(newPlayer, sockFD, syscall(__NR_gettid));
}

//PLAYERS DATA FUNCTIONS

void askName(int sockFD, char buffer[], char name[NAMESIZE])
{
    askMSG(sockFD, "Inserisci il tuo nome: ");
    read(sockFD, buffer, BUFFSIZE);
    strcpy_noNL(name, buffer); 
}

void getPlayerName(int sockFD, char buffer[], char name[NAMESIZE])
{
    setBuff(buffer, "");

    askName(sockFD, buffer, name);
    while(! ValidName(name))
        {
            sendErrorMSG(sockFD, wrongInput, "name not valid, must be a word with Capital initial and no numbers\n");
            read(sockFD, buffer, BUFFSIZE); setBuff(buffer, ""); 
            askName(sockFD, buffer, name);
        }
             
}

void askNumber(int sockFD, char buffer[], char sNum[5])
{
    askMSG(sockFD, "Inserisci il tuo numero di maglia: ");
    read(sockFD, buffer, BUFFSIZE); strcpy_noNL(sNum, buffer);
}

int getPlayerNumber(int sockFD, char buffer[], char sNum[5])
{
    setBuff(buffer, "");

    askNumber(sockFD, buffer, sNum);
    while(! ValidNumber(sNum))
    {
        sendErrorMSG(sockFD, wrongInput, "number not valid, must in between 1 and 99\n");
        read(sockFD, buffer, BUFFSIZE); setBuff(buffer, ""); 
        askNumber(sockFD, buffer, sNum);
    }

    int num = atoi(sNum); 
        
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
        default : sendErrorMSG(sockFD, wrongInput, "only accepted answers are Y and N\n"); read(sockFD, buffer, BUFFSIZE);
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

int answA =  -1, answB = -1;

void* AskCaptain(void* Player)
{
    struct player* currPlayer = (struct player*) Player; int* answ;

    if(currPlayer->team == 'A') answ = &answA;
    else if (currPlayer->team == 'B') answ = &answB;
    else {printf("Non c'è il carattere team\n") ;pthread_exit(NULL);}
    
    char buffer[BUFFSIZE];
    askMSG(currPlayer->FD, "Do you want to accept the rematch? (Y/N): ");
    read(currPlayer->FD, buffer, BUFFSIZE);
    int loop = 1;
    while(loop)
    {
        switch (buffer[0])
        {
        case 'Y':
            *answ = 1; loop = 0;
            break;
        case 'N':
            *answ = 0;  loop = 0;
            break;
        default:
            sendErrorMSG(currPlayer->FD, wrongInput, "only accepted answers are Y and N\n"); read(currPlayer->FD, buffer, BUFFSIZE);
            setBuff(buffer, "");
            askMSG(currPlayer->FD, "Do you want to accept the rematch? (Y/N): "); read(currPlayer->FD, buffer, BUFFSIZE);
            break;
        }
    }   
}

void RestartGame(struct referee* Ref)
{
    sendMSGtoAllClients(*Ref, "Rematch is ON! Restarting the game...\n");
    printf("Rematch is ON! Restarting the game...\n");
    ResetRef(Ref);
}

void DisbandGame(struct referee* Ref)
{
    Ref->gameStatus = gameDisbanded;
    printf("\nEnd of Match!\n"); writeLog(Ref->pathLogServer, "\nEnd of Match!\n");
}

int AskForRematch(struct referee* Ref)
{
    sendMSGtoAllClients(*Ref, "Asking captains for a rematch\n");
    printf("Asking captains for a rematch\n");
    pthread_t a, b;
    pthread_create(&a, NULL, AskCaptain, (void*) Ref->teamA.captain);
    pthread_create(&a, NULL, AskCaptain, (void*) Ref->teamB.captain);
    
    while(answA == -1 && answB == -1);

    if(answA && answB)
        RestartGame(Ref);
    else
        DisbandGame(Ref);
}

#endif
