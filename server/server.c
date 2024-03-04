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
#define QSIZE 15

struct referee Ref;
struct player playerQueueA[QSIZE]; short addedIndexA = 0, viewedIndexA = 0;
struct player playerQueueB[QSIZE]; short addedIndexB = 0, viewedIndexB = 0;

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

void printStatusMatch(int sockFD, char buffer[], struct player* currPlayer, char team)
{    

    if (team == 'A')
    {
        setBuff(buffer, "Sei il capitano della squadra A\n");
        sendMSG(sockFD, buffer); read(sockFD, buffer, BUFFSIZE);
        
            setBuff(buffer, "");

        askMSG(sockFD, "Inserisci nome Squadra A: "); 
        read(sockFD, buffer, BUFFSIZE); strcpy_noNL(Ref.teamA.teamName, buffer);
        printf("Il primo team è %s\n", Ref.teamA.teamName);     
    }

    if (team == 'B')
    {
        setBuff(buffer, "Sei il capitano della squadra B\n");
        sendMSG(sockFD, buffer); read(sockFD, buffer, BUFFSIZE);
        
            setBuff(buffer, "");

        askMSG(sockFD, "Inserisci nome Squadra B: ");
        read(sockFD, buffer, BUFFSIZE); strcpy_noNL(Ref.teamB.teamName, buffer);
        printf("Il secondo team è %s\n", Ref.teamB.teamName);
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


    if (team == 'A') { initPlayer(currPlayer, name, num, Ref.teamA.teamName); }
    if (team == 'B') { initPlayer(currPlayer, name, num, Ref.teamB.teamName); }

    currPlayer->playerFD = sockFD;
    currPlayer->playerTID = syscall(__NR_gettid);


    setBuff(buffer, "Il capitano della squadra "); strcat(buffer, currPlayer->teamName);
    strcat(buffer, " è "); strcat(buffer, currPlayer->name);
    strcat(buffer, " con numero "); strcat(buffer, sNum); strcat(buffer, "\n");
    
    sendMSG(sockFD, buffer); read(sockFD, buffer, BUFFSIZE);
    printf("%s", buffer);

        setBuff(buffer, "");

    

    while(Ref.gameStatus == oneCaptainNeeded);
    Ref.gameStatus--;

    while(Ref.gameStatus != gameCreated);
}


void* AcceptNewPlayer(void* socketFD)
{
    int sockFD = *((int*) socketFD);
    struct player* currPlayer;
    char buffer[BUFFSIZE] = "";
    if(Ref.gameStatus == nogame)
    {
        Ref.gameStatus = oneCaptainNeeded;

            currPlayer = Ref.teamA.captain;

            sendMSG(sockFD, "Inizio creazione partita\n\n"); read(sockFD, buffer, BUFFSIZE);
            setBuff(buffer, "");

            printStatusMatch(sockFD, buffer, currPlayer, 'A');
            Ref.teamA.membNum++;


            setBuff(buffer, "La partita è ");strcat(buffer, Ref.teamA.teamName);
            strcat(buffer, "-");
            strcat(buffer, Ref.teamB.teamName);strcat(buffer, "\n");
            
            
            printf("%s", buffer);
            sendMSG(sockFD, buffer); read(sockFD, buffer, BUFFSIZE);
            

            //capitano accetta giocatori
            while(Ref.teamA.membNum < TEAMSIZE)
            {
                while(addedIndexA == viewedIndexA);

                sendMSG(sockFD, "Un giocatore ha richiesto di entrare nella tua squadra\n"); read(sockFD, buffer, BUFFSIZE);
                setBuff(buffer, playerQueueA[viewedIndexA].name); strcat(buffer, ", lo vuoi accettare? (Y/N)\n");
                askMSG(sockFD, buffer); read(sockFD, buffer, BUFFSIZE);
                char answer = buffer[0];

                if(answer == 'Y')
                {
                    Ref.teamA.members[Ref.teamA.membNum] = playerQueueA[viewedIndexA];
                    Ref.teamA.membNum++;
                    sendMSG(playerQueueA[viewedIndexA].playerFD, "Accettato nella squadra A");
                    read(playerQueueA[viewedIndexA].playerFD, buffer, BUFFSIZE);
                    sendMSG(sockFD, "Giocatore accettato\n"); read(sockFD, buffer, BUFFSIZE);


                }
                else if(answer == 'N')
                {
                    sendMSG(playerQueueA[viewedIndexA].playerFD, "Rifiutato dalla squadra A");
                    sendMSG(sockFD, "Giocatore rifiutato\n");
                }

                viewedIndexA++;




            }
    
    }
    else if(Ref.gameStatus == oneCaptainNeeded )
    {
        Ref.gameStatus--;
            currPlayer = Ref.teamB.captain;

            printStatusMatch(sockFD, buffer, currPlayer, 'B');
            Ref.teamB.membNum++;
            

            setBuff(buffer, "La partita è ");strcat(buffer, Ref.teamA.teamName);
            strcat(buffer, "-");
            strcat(buffer, Ref.teamB.teamName);strcat(buffer, "\n");

            
            sendMSG(sockFD, buffer); read(sockFD, buffer, BUFFSIZE);

                setBuff(buffer, "");

            //capitano accetta giocatori
            while(Ref.teamB.membNum < TEAMSIZE)
            {
                while(addedIndexB == viewedIndexB);

                sendMSG(sockFD, "Un giocatore ha richiesto di entrare nella tua squadra\n"); read(sockFD, buffer, BUFFSIZE);
                setBuff(buffer, playerQueueB[viewedIndexB].name); strcat(buffer, ", lo vuoi accettare? (Y/N)\n");
                askMSG(sockFD, buffer); read(sockFD, buffer, BUFFSIZE);
                char answer = buffer[0];

                if(answer == 'Y')
                {
                    Ref.teamB.members[Ref.teamB.membNum] = playerQueueB[viewedIndexB];
                    Ref.teamB.membNum++;
                    sendMSG(playerQueueB[viewedIndexB].playerFD, "Accettato nella squadra B");
                    sendMSG(sockFD, "Giocatore accettato\n");


                }
                else if(answer == 'N')
                {
                    sendMSG(playerQueueB[viewedIndexB].playerFD, "Rifiutato dalla squadra B");
                    sendMSG(sockFD, "Giocatore rifiutato\n");
                }

                viewedIndexB++;

            }
    
    }
    else 
    {
        if(Ref.gameStatus == gameCreation || Ref.gameStatus == waitingOtherCaptain)
        {
            sendMSG(sockFD, "Creazione partita in corso: ATTENDI\n\n");
            read(sockFD, buffer, BUFFSIZE);

            while(Ref.gameStatus != gameCreated);
            sendMSG(sockFD, "Partita creata!\n");
            read(sockFD, buffer, BUFFSIZE);
        }
        else if(Ref.gameStatus == gameCreated)
        {
            sendMSG(sockFD, "Creazione partita completata\n\n");
            read(sockFD, buffer, BUFFSIZE);
        }

        sendMSG(sockFD, "Crea il tuo giocatore!\n"); read(sockFD, buffer, BUFFSIZE);
        
        struct player tmpPlayer;
        char ansBuff[BUFFSIZE] = "";

            setBuff(buffer, "");

        askMSG(sockFD, "Inserisci il tuo nome: ");
        read(sockFD, buffer, BUFFSIZE);
        char name[50]; strcpy_noNL(name, buffer);

            setBuff(buffer, "");

        askMSG(sockFD, "Inserisci il tuo numero di maglia: ");
        read(sockFD, buffer, BUFFSIZE); int num = atoi(buffer);
        char sNum[3] = ""; strcpy_noNL(sNum, buffer);
        
            setBuff(buffer, "");
       
        
        initPlayer(&tmpPlayer, name, num, NULL);
        tmpPlayer.playerFD = sockFD;
        tmpPlayer.playerTID = syscall(__NR_gettid);

            setBuff(buffer, "");
        
        setBuff(buffer, "Nuovo giocatore: "); strcat(buffer, tmpPlayer.name);
        strcat(buffer, " con numero "); strcat(buffer, sNum); strcat(buffer, "\n");
        printf("%s", buffer);
        sendMSG(sockFD, buffer); read(sockFD, buffer, BUFFSIZE); 

            setBuff(buffer, "");

        //nuovo giocatore si pone in coda per entrare in una squadra
        
        setBuff(buffer, "1-> "); strcat(buffer, Ref.teamA.teamName);
        strcat(buffer, "; 2-> "); strcat(buffer, Ref.teamB.teamName);
        strcat(buffer, "\n"); sendMSG(sockFD, buffer); read(sockFD, buffer, BUFFSIZE);

        setBuff(buffer, "");

        
        askMSG(sockFD, "Decidi di che squadra far parte: "); 
        read(sockFD, ansBuff, BUFFSIZE);
        char teamChoice = atoi(ansBuff);

        if(teamChoice == 1)
        {
            playerQueueA[addedIndexA] = tmpPlayer;
            addedIndexA = (addedIndexA+1)%QSIZE;
        }
        else if(teamChoice == 2)
        {  
            playerQueueB[addedIndexB] = tmpPlayer;
            addedIndexB = (addedIndexB+1)%QSIZE;
        }

        read(sockFD, buffer, BUFFSIZE);


        
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