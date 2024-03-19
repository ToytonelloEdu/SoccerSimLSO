#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <sys/stat.h>

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
#include "../lib/serverManagement.h"
#include "../lib/clientManagement.h"


int S = 1;
struct referee Ref; int pipe_A[2]; int pipe_B[2];
struct playerQueue QueueA = {0,0}, QueueB = {0,0};

    void selectAction(struct player* player)
    {
        wait(&S);
        
            char buffer[BUFFSIZE] = "";
            PlayerToString(buffer, *player);

            switch (getRandomAction(defaultProbs))
            {
            case eShot:      
                Shot(&Ref, player, buffer, defShotProb);
                break;
            case eDribble: 
                Dribbling(&Ref, player, buffer, defDribbleProb);           
                break;
            case eInjury:    
                Injury(&Ref, player, buffer);            
                break;
            default: 
                sendErrorMSG(player->FD, wrongInput, "");
                break;
            }

            sleep(ACT_INTERVAL);

        signal(&S);
        sleep(ACT_COOLDOWN);
    }

    void* AcceptNewPlayer(void* socketFD)
    {
        int sockFD = *((int*) socketFD);
        struct player* currPlayer = 0;
        char buffer[BUFFSIZE] = "";

        if(Ref.gameStatus == nogame)
            {
                Ref.gameStatus = oneCaptainNeeded;
                    currPlayer = Ref.teamA.captain;

                    sendMSG(sockFD, "Inizio creazione partita\n\n"); read(sockFD, buffer, BUFFSIZE);
                        setBuff(buffer, "");

                    TeamCaptainInitialization(sockFD, buffer,&Ref, currPlayer, 'A');
                    Ref.teamA.membNum++;

                    MatchPresentation(sockFD, buffer, Ref);
                    printf("%s", buffer);
                    
                        setBuff(buffer, "");
                    
                    TeamMemberAcceptance(sockFD, buffer, & Ref.teamA, "A", & QueueA, pipe_A[1]); //capitano accetta giocatori

                    sendMSG(sockFD, "Squadra al completo!\n"); read(sockFD, buffer, BUFFSIZE);
                    while(Ref.teamB.membNum < TEAMSIZE);

                Ref.gameStatus = gameStarting;

            }
        else if(Ref.gameStatus == oneCaptainNeeded )
            {
                Ref.gameStatus--;
                    currPlayer = Ref.teamB.captain;

                    TeamCaptainInitialization(sockFD, buffer,&Ref, currPlayer, 'B');
                    Ref.teamB.membNum++;
                    
                    MatchPresentation(sockFD, buffer, Ref);

                        setBuff(buffer, "");

                    TeamMemberAcceptance(sockFD, buffer, & Ref.teamB, "B", & QueueB, pipe_B[1]); //capitano accetta giocatori

                    sendMSG(sockFD, "Squadra al completo!\n"); read(sockFD, buffer, BUFFSIZE);
                    
                while(Ref.teamA.membNum < TEAMSIZE);
            }
        else 
            {
                WaitCaptains(sockFD, buffer, &Ref);
                MatchPresentation(sockFD, buffer, Ref);

                    sendMSG(sockFD, "\nCrea il tuo giocatore!\n"); read(sockFD, buffer, BUFFSIZE);
                
                struct player tmpPlayer;
                NewPlayerInitialization(sockFD, buffer, &tmpPlayer);
                playerConfirmation(sockFD, buffer, &tmpPlayer);

                    setBuff(buffer, "");
                
                switch (TeamRequestChoice(sockFD, buffer, & Ref)) //nuovo giocatore si pone in coda per entrare in una squadra
                {
                case 1 : currPlayer = TeamMemberRequest(sockFD, & Ref, & tmpPlayer, & QueueA, pipe_A[0]);
                    break;
                case 2 : currPlayer = TeamMemberRequest(sockFD, & Ref, & tmpPlayer, & QueueB, pipe_B[0]);
                    break;
                default : sendErrorMSG(sockFD, wrongInput, "choose between teams A and B");
                    break;
                }

                WaitFullTeams(sockFD, buffer, &Ref);
            }
        
        sendMSG(sockFD, "Tutto pronto: INIZIA LA PARTITA\n\n"); read(sockFD, buffer, BUFFSIZE);    

        while(Ref.time < 0);

        while(Ref.time < DURATION){
            if (currPlayer -> resumePlay <= DURATION)
            {
                while(Ref.time < currPlayer->resumePlay && Ref.gameStatus == gameStarted);
                selectAction(currPlayer);
            }
        }
    }

    int createNewLogFile()
    {
        char pathLogServer[100], pathLogLib[100], buffer[26];
        time_t hour; time(&hour); ctime_r(&hour, buffer);
        
        sprintf(pathLogServer, "./log/%s-logFile.txt", buffer);    
        strncpy(Ref.pathLogServer, pathLogServer, 54);

        return creat(pathLogServer, S_IRUSR | S_IWUSR | S_IROTH);
    }

    void* MatchClockThread(void* arg)
    {
        while(Ref.gameStatus != gameStarting);

        Ref.logFD = createNewLogFile();
        char buffer[BUFFSIZE] = "";

        MatchStart(&Ref);
            while(Ref.time < DURATION)
            {
                Ref.time++;
                sendMinuteToAllOutputs(Ref);
                sleep(2);
            }
        RecoveryTime(buffer, &Ref);

        sleep(5);
        wait(&S);
            MatchFinish(buffer, &Ref);            
        signal(&S);
    }

    int main(int argc, char* argv[])
    {
        printf("Welcome to LSOccer Simulator's Server!\n");

        int wsock_fd, new_socket;
        int opt = 1;
        struct sockaddr_in servaddr, cliaddr;
        createPipe(pipe_A); createPipe(pipe_B); srand(time(NULL));
        InitReferee(&Ref);initTeam(&(Ref.teamA));initTeam(&(Ref.teamB));
        pthread_create(& Ref.clockThread, NULL, MatchClockThread, NULL);
        pthread_detach(Ref.clockThread); pthread_setschedprio(Ref.clockThread, 10);
        
        if((wsock_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        { perror("Failed to create socket"); exit(EXIT_FAILURE); }

        if(setsockopt(wsock_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
        { perror("setsockopt failed"); exit(EXIT_FAILURE); }

        memset(&servaddr, 0, sizeof(servaddr)); memset(&cliaddr, 0, sizeof(cliaddr));

        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(12345);
        servaddr.sin_addr.s_addr = INADDR_ANY;
        
        if(bind(wsock_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
        { perror("Bind failed"); exit(EXIT_FAILURE); }

        
        if(listen(wsock_fd, 10) < 0)
        { perror("listen error"); exit(EXIT_FAILURE); }

        int addrlen = sizeof(cliaddr);

        while((new_socket = accept(wsock_fd, (struct sockaddr*)&cliaddr, (socklen_t*)&addrlen)) > -1)
        {
            printf("New player connected\n");
            pthread_create(&Ref.lastThread, NULL, AcceptNewPlayer, (void*) &new_socket);
            pthread_detach(Ref.lastThread);
        }

        close(wsock_fd);

        return 0;
    }