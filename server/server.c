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
#include "../lib/clientManagement.h"
#include "../lib/serverManagement.h"

int S = 1;
struct referee Ref; int pipe_A[2]; int pipe_B[2];
struct playerQueue QueueA = {0,0}, QueueB = {0,0};

enum actions {eShot, eDribbling, eInjury};

    void selectAction(struct player* player)
    {
        wait(&S);
        
        char buffer[BUFFSIZE] = "";
        PlayerToString(buffer, *player);
        int result = rand() % 3;

        if (result == eShot ) { shot(&Ref, player, buffer); } 
        if (result == eDribbling ) { dribbling(&Ref, player, buffer); }
        if (result == eInjury ) { injury(&Ref, player, buffer); }

        delay(15000);

        signal(&S);
        delay(500);
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

                setBuff(buffer, "La partita è ");strcat(buffer, Ref.teamA.teamName);
                strcat(buffer, "-");
                strcat(buffer, Ref.teamB.teamName);strcat(buffer, "\n");
                
                printf("%s", buffer);
                sendMSG(sockFD, buffer); read(sockFD, buffer, BUFFSIZE);
                
                //capitano accetta giocatori
            
                TeamMemberAcceptance(sockFD, buffer, & Ref.teamA, "A", & QueueA, pipe_A[1]);

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
                

                setBuff(buffer, "La partita è ");strcat(buffer, Ref.teamA.teamName);
                strcat(buffer, "-");
                strcat(buffer, Ref.teamB.teamName);strcat(buffer, "\n");

                
                sendMSG(sockFD, buffer); read(sockFD, buffer, BUFFSIZE);

                    setBuff(buffer, "");

                //capitano accetta giocatori
                
                TeamMemberAcceptance(sockFD, buffer, & Ref.teamB, "B", & QueueB, pipe_B[1]);

                sendMSG(sockFD, "Squadra al completo!\n"); read(sockFD, buffer, BUFFSIZE);
                
            while(Ref.teamA.membNum < TEAMSIZE);
        
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
        
            
            initPlayer(&tmpPlayer, name, num);
            tmpPlayer.playerFD = sockFD;
            tmpPlayer.playerTID = syscall(__NR_gettid);

                setBuff(buffer, "");
            
            setBuff(buffer, "Nuovo giocatore: "); strcat(buffer, tmpPlayer.name);
            strcat(buffer, " con numero "); strcat(buffer, sNum); strcat(buffer, "\n");
            printf("%s", buffer);
            sendMSG(sockFD, buffer); read(sockFD, buffer, BUFFSIZE); 

                setBuff(buffer, "");

            //nuovo giocatore si pone in coda per entrare in una squadra
            
            switch (TeamRequestChoice(sockFD, buffer, & Ref))
            {
            case 1 :
                TeamMemberRequest(sockFD, & Ref, & tmpPlayer, & QueueA, pipe_A[0]);
                break;
            
            case 2:
                TeamMemberRequest(sockFD, & Ref, & tmpPlayer, & QueueB, pipe_B[0]);
                break;
            default:
                sendErrorMSG();
                break;
            }

            if(Ref.gameStatus < gameStarting)
            {
                sendMSG(sockFD, "Squadre non al completo, attendi inizio partita\n"); read(sockFD, buffer, BUFFSIZE);
                while(Ref.gameStatus != gameStarting);
            }
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
        time_t hour;
        time(&hour);
        ctime_r(&hour, buffer);
        
        sprintf(pathLogLib, "../server/log/%s-logFile.txt", buffer);
        strncpy(Ref.pathLogLib, pathLogLib, 54);
        
        sprintf(pathLogServer, "./log/%s-logFile.txt", buffer);    
        strncpy(Ref.pathLogServer, pathLogServer, 54);
        return creat(pathLogServer, S_IRUSR | S_IWUSR | S_IROTH);
    }

    void* MatchClockThread(void* arg)
    {
        while(Ref.gameStatus != gameStarting);
        Ref.logFD = createNewLogFile();

        printf("\nTutto pronto: INIZIA LA PARTITA\n\n");
        delay(500);
        printf("Minute: 0\n");

        int i = 0; 
        char tmpBuff[BUFFSIZE] = "";
        sprintf(tmpBuff, "Minute: 0\n");
        writeLog(Ref.pathLogServer, tmpBuff);
        setBuff(tmpBuff, "");
        Ref.gameStatus = gameStarted; Ref.time = 0;
            
        sendMinuteToAllClients(Ref);
        while(Ref.time < DURATION)
        {
            printf("Minute: %d\n", ++Ref.time);
            sprintf(tmpBuff, "Minute: %d\n", Ref.time);
            writeLog(Ref.pathLogServer, tmpBuff);
            setBuff(tmpBuff, "");
            sendMinuteToAllClients(Ref);
            sleep(2);
        }

        printf("Minuti di recupero\n");
        sprintf(tmpBuff, "Minuti di recupero\n");
        writeLog(Ref.pathLogServer, tmpBuff);
        setBuff(tmpBuff, "");
        sendMSGtoAllClients(Ref, "Minuti di recupero\n");

        wait(&S);
            char buffer[BUFFSIZE] = "";
            sprintf(buffer, "\n\nPARTITA FINITA\n%s %d-%d %s\n", Ref.teamA.teamName, Ref.stats.numberGoalA, Ref.stats.numberGoalB, Ref.teamB.teamName);
            printf("%s", buffer); sendMSGtoAllClients(Ref, buffer);
            printStatsOfMAtch(Ref);            
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
        pthread_detach(Ref.clockThread);
        pthread_setschedprio(Ref.clockThread, 10);
        
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
        servaddr.sin_port = htons(12345);
        servaddr.sin_addr.s_addr = INADDR_ANY;
        
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

        close(wsock_fd);

        return 0;
    }