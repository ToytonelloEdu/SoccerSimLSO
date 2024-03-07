#ifndef GLFUNCTS
#define GLFUNCTS

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include "gameLogicStructs.h"
#include "clientManagement.h"

#define DURATION 10

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
        { strcat(msg, "\n"); shot(Ref, player, msg); }
        else 
        {
            strcat(msg, " failed\n");
            printf("%s", msg);
            sendMSGtoAllClients(*Ref, msg);
            //messaggio dribbling fallito
        }
    }

    void shot(struct referee* Ref, struct player* player, char* msg)
    {
        int result = rand() % 2;    //0 GOAL -- 1 NO GOAL

        strcat(msg, "\tShot");

        if (result == 0) 
            { strcat(msg, "\n"); goal(Ref, player, msg); } 
        else
            { 
                addShotFailed(&(Ref -> stats)); 
                strcat(msg, " failed\n");
                printf("%s", msg);
                sendMSGtoAllClients(*Ref, msg);
            }
    }

    void goal(struct referee* Ref, struct player* player, char* msg)
    {
        if(player->team == 'A') {Ref->stats.numberGoalA++;}
        else if(player->team == 'B') {Ref->stats.numberGoalB++;}

        int FD = Ref -> logFD;

        sprintf(msg, "%s\tGOAL: %d-%d\n", msg, Ref->stats.numberGoalA, Ref->stats.numberGoalB);
        printf("%s", msg);
        sendMSGtoAllClients(*Ref, msg);
    }

    void injury(struct referee* Ref, struct player* player, char* msg)
    {
        strcat(msg, "\tInjury\n");
        printf("%s", msg);
        sendMSGtoAllClients(*Ref, msg);
    }

    
    
    void addDribbling(struct stats* stats) { stats -> numberDribbling++; }
    void addShotFailed(struct stats* stats) { stats -> shotFailed++; }



#endif