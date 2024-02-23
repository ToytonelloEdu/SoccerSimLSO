
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

#include "gameLogicStructs.h"
#include "gameLogicFuncts.h"

#define DURATION 15

ball S = 1;
struct referee Ref;
enum actions {eShot, eInjury, eDribbling};


    void selectAction(struct player* player)
    {
        wait(&S);
                
        int result = rand() % 3;

        printPlayer(player);
        if (result == eShot ) { shot(&Ref, player); } 
        if (result == eInjury ) { injury(&Ref, player); }
        if (result == eDribbling ) { dribbling(&Ref, player); }

        delay(7500);

        signal(&S);
        delay(200);
    }

    

    void* startClientThread(void* player)
    {
        while(Ref.time < 0);

        while(Ref.time < DURATION)
            selectAction((struct player*) player);
    }

    int main()
    {
        srand(time(NULL));
        InitReferee(&Ref);
        initTeam(&(Ref.teamA));
        initTeam(&(Ref.teamB));
        Ref.teamA.teamName = "Nemezis";
        Ref.teamB.teamName = "CR7evens";

        initPlayer(Ref.teamA.captain, "Messi", 10, &(Ref.teamA.teamName));
        Ref.teamA.members[0].playerFD = 2;

        initPlayer(Ref.teamB.captain, "Ronaldo", 7, &(Ref.teamB.teamName));
        Ref.teamB.members[0].playerFD = 3;
        
        pthread_t* tid1 = &(Ref.teamA.members[0].playerTID);
        pthread_t* tid2 = &(Ref.teamB.members[0].playerTID);
        
        pthread_create(tid1, NULL, startClientThread, (void*) Ref.teamA.captain);
        //sleep(1);
        pthread_create(tid2, NULL, startClientThread, (void*) Ref.teamB.captain);

        
        printf("Minute: 0\n");
        Ref.time = 0;
        while(Ref.time < DURATION)
        {
            printf("Minute: %d\n", ++Ref.time);
            sleep(2);
        }

        return 0;
    }