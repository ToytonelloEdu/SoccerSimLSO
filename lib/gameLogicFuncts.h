#ifndef GLFUNCTS
#define GLFUNCTS

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include "gameLogicStructs.h"

void shot(struct referee* Ref, struct player* player);
void injury(struct referee* Ref, struct player* player);
void dribbling(struct referee* Ref, struct player* player);
void goal(struct referee* Ref, struct player* player);
void addShotFailed(struct stats* stats);
void addDribbling(struct stats* stats);
typedef int ball;

    void wait(ball* mutex)
    {
        while(*mutex <= 0);
        
        (*mutex)--;
    }

    void signal(ball* mutex)
    {
        (*mutex)++;
    }

    void delay(int num_of_seconds)
    {
        int milliseconds = 1000 * num_of_seconds;
        clock_t start = clock();
        while(clock() < start + milliseconds){}
    }

    void shot(struct referee* Ref, struct player* player)
    {
        int result = rand() % 2;    //0 GOAL -- 1 NO GOAL
        printf("\tShot\n");
        if (result == 0) 
        { goal(Ref, player); } 
        else
        { addShotFailed(&(Ref -> stats)); }
    }

    void injury(struct referee* Ref, struct player* player)
    {
        printf("\tInjury\n");
    }

    void dribbling(struct referee* Ref, struct player* player)
    {
        int result = rand() % 2;    //0 SUCCESS -- 1 FAILED

        addDribbling(&(Ref -> stats));
        printf("\tDribbling\n");

        if (result == 0) { shot(Ref, player); }
    }

    void goal(struct referee* Ref, struct player* player)
    {
        Ref -> stats.numberGoal++;
        int FD = Ref -> logFD;
        printf("\tGOAL\n");

    }
    
    void addDribbling(struct stats* stats) { stats -> numberDribbling++; }
    void addShotFailed(struct stats* stats) { stats -> shotFailed++; }



#endif