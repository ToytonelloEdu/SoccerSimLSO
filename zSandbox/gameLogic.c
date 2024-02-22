
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

#define DURATION 10

ball S = 1;
struct referee Ref;
enum actions {eShot, eInjury, eDribbling};


    void selectAction(struct player* player)
    {
        printf("%d\n", *(&S));
        wait(&S);
        printf("%d\n", *(&S));
        
        int result = rand() % 3;
        
        if (result == eShot ) { shot(&Ref, player); } 
        if (result == eInjury ) { injury(&Ref, player); }
        if (result == eDribbling ) { dribbling(&Ref, player); }

        delay(5000);

        signal(&S);
        printf("%d\n", *(&S));
    }

    

    void* startClientThread(void* player)
    {
        while(Ref.time < DURATION)
            selectAction((struct player*) player);
    }

    int main()
    {
        srand(time(NULL));
        initTeam(Ref.teamA);
        initTeam(Ref.teamB);

        Ref.teamA.members[0].name = "Messi";
        Ref.teamA.members[0].shirtNumber = 10;
        Ref.teamA.members[0].playerFD = 2;

        Ref.teamB.members[0].name = "Ronaldo";
        Ref.teamB.members[0].shirtNumber = 7;
        Ref.teamB.members[0].playerFD = 3;

        initTeam(Ref.teamA);
        initTeam(Ref.teamB);
        
        
        pthread_t tid1 = Ref.teamA.members[0].playerTID;
        pthread_t tid2 = Ref.teamB.members[0].playerTID;
        
        pthread_create(&(tid1), NULL, startClientThread, (void*) Ref.teamA.captain);
        pthread_create(&(tid2), NULL, startClientThread, (void*) Ref.teamB.captain);

        Ref.time = 0;
        while(Ref.time < DURATION)
        {
            Ref.time++;
            sleep(2);
        }

        return 0;
    }