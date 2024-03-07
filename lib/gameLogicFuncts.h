#ifndef GLFUNCTS
#define GLFUNCTS

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include "gameLogicStructs.h"
#include "clientManagement.h"

#define DURATION 90

void dribbling(struct referee* Ref, struct player* player, char* msg);
void shot(struct referee* Ref, struct player* player, char* msg);
void goal(struct referee* Ref, struct player* player, char* msg);
void injury(struct referee* Ref, struct player* player, char* msg);
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

    void dribbling(struct referee* Ref, struct player* player, char* msg)
    {
        int result = rand() % 2;    //0 SUCCESS -- 1 FAILED

        addDribbling(&(Ref -> stats));
        strcat(msg, "\tDribbling");

        if (result == 0) 
        { shot(Ref, player, msg); }
        else 
        {
            strcat(msg, " Failed\n");
            printf("%s", msg);
            //messaggio dribbling fallito
        }
    }

    void shot(struct referee* Ref, struct player* player, char* msg)
    {
        int result = rand() % 2;    //0 GOAL -- 1 NO GOAL

        strcat(msg, "\tShot\n");

        if (result == 0) 
            { goal(Ref, player, msg); } 
        else
            { 
                addShotFailed(&(Ref -> stats)); 
                strcat(msg, " Failed\n");
                printf("%s", msg);
            }
    }

    void goal(struct referee* Ref, struct player* player, char* msg)
    {
        player->team == 'A'?Ref->stats.numberGoalA++ : Ref->stats.numberGoalB++;
        int FD = Ref -> logFD;

        sprintf(msg, "\tGOAL: %d-%d\n", Ref->stats.numberGoalA, Ref->stats.numberGoalB);
        printf("%s", msg);
        

    }

    void injury(struct referee* Ref, struct player* player, char* msg)
    {
        printf("\tInjury\n");
    }

    
    
    void addDribbling(struct stats* stats) { stats -> numberDribbling++; }
    void addShotFailed(struct stats* stats) { stats -> shotFailed++; }



#endif