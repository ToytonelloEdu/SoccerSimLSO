#ifndef GLFUNCTS
#define GLFUNCTS

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include "gameLogicStructs.h"
#include "clientManagement.h"
#include "serverManagement.h"

#define DURATION 90
#define ACT_INTERVAL 5
#define ACT_COOLDOWN 2
#define INJ_TIME_MOD 20
#define INJ_TIME_BASE 5
#define PEN_TIME_MOD 20
#define PEN_TIME_BASE 5

struct actsProb;
struct resProb;

void Dribbling(struct referee* Ref, struct player* player, char* msg, struct resProb resP);
void Shot(struct referee* Ref, struct player* player, char* msg, struct resProb resP);
void Goal(struct referee* Ref, struct player* player, char* msg);
void Injury(struct referee* Ref, struct player* player, char* msg);
void Penalize(struct referee* Ref, char oppTeam, char* msg);
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

    int valInBetween(int bottom, int val, int top)
    {
        if(val > bottom && val <= top) return 1;

        return 0;
    }

    enum actions {eShot, eDribble, eInjury};

    struct actsProb
    {
        int ShotProb;
        int DribbleProb;
        int InjuryProb;
        int Denom;
    };

    #define SHOT_PROB 30
    #define DRIB_PROB 65
    #define INJ_PROB 5


    struct actsProb defaultProbs = {SHOT_PROB, DRIB_PROB, INJ_PROB, 100};

    enum actions getRandomAction(struct actsProb actP)
    {
        int bottom, top, val = (rand() % actP.Denom) + 1;
        
        bottom = 0; top = actP.ShotProb;
        if(valInBetween(bottom, val, top)) return eShot;

        bottom += actP.ShotProb; top += actP.DribbleProb;
        if(valInBetween(bottom, val, top)) return eDribble;

        bottom += actP.DribbleProb; top += actP.InjuryProb;
        if(valInBetween(bottom, val, top)) return eInjury;

        return -1;
    }

    enum actResult {failure, success};

    struct resProb
    {
        int Numer;
        int Denom;
    };

    enum actResult getActionResult(struct resProb resP)
    {
        int val = (rand() % resP.Denom) + 1;

        if(valInBetween(0, val, resP.Numer)) return success;

        return failure;
    }

    struct resProb defDribbleProb = {2,5};
    struct resProb defShotProb = {1,5};

    void Dribbling(struct referee* Ref, struct player* player, char* msg, struct resProb resP)
    {
        enum actResult result = getActionResult(resP);
        struct resProb shotRP = {2,5};

        addDribbling(&(Ref -> stats)); strcat(msg, "\tDribbling");

        if (result == success) 
        { strcat(msg, "\n"); Shot(Ref, player, msg, shotRP);}
        else 
        {
            strcat(msg, " failed\n");
            sendMSGtoAllOutputs(*Ref, msg);
        }
    }

    void Shot(struct referee* Ref, struct player* player, char* msg, struct resProb resP)
    {
        enum actResult result = getActionResult(resP);

        strcat(msg, "\tShot");

        if (result == success) 
            { strcat(msg, "\n"); Goal(Ref, player, msg); } 
        else
        {
            addShotFailed(&(Ref -> stats));
            strcat(msg, " failed\n"); sendMSGtoAllOutputs(*Ref, msg);
        }
    }

    void Goal(struct referee* Ref, struct player* player, char* msg)
    {
        if(player->team == 'A') {Ref->stats.numberGoalA++;}
        else if(player->team == 'B') {Ref->stats.numberGoalB++;}

        sprintf(msg, "%s\tGOAL: %d-%d\n", msg, Ref->stats.numberGoalA, Ref->stats.numberGoalB);
        sendMSGtoAllOutputs(*Ref, msg);
    }

    int getInjuryTime()
    {
        return (rand() % INJ_TIME_MOD + 1) + INJ_TIME_BASE;
    }

    void getInjuryMSG(char* msg, struct player* player, int mins)
    {
        if(player->resumePlay <= DURATION)
            {sprintf(msg, "%s\tInjured until min. %d (for %d minutes)\n", msg, player->resumePlay, mins);} 
        else 
            {sprintf(msg, "%s\tInjured until end of the game.\n", msg);}
    }

    void Injury(struct referee* Ref, struct player* player, char* msg)
    {
        int mins = getInjuryTime();
        
        player->resumePlay = (Ref->time) + mins;

        getInjuryMSG(msg, player, mins);
        Penalize(Ref, player->team, msg);
        sendMSGtoAllOutputs(*Ref, msg);
    }

    int getPenaltyTime()
    {
        return (rand() % PEN_TIME_MOD + 1) + PEN_TIME_BASE;
    }

    int getPlayerBadIndex(int* time, struct team* team_getFrom)
    {
        int index = 0;
        while(1)
        {
            index = rand() % TEAMSIZE;
            struct player* player = & team_getFrom->members[index];
            if(player->resumePlay < *time)
            {
                return index;
            }
        }
    }

    void getPenaltyMSG(char* msg, struct player* player, int mins)
    {
        if(player->resumePlay <= DURATION)
            {sprintf(msg, "%s\tPlayer %s penalized until min. %d (for %d minutes)\n", msg, player->name, player->resumePlay, mins);} 
        else 
            {sprintf(msg, "%s\tPlayer %s expelled from the game.\n", msg, player->name);}
    }    

    void Penalize(struct referee* Ref, char oppTeam, char* msg)
    {
        int playerBad_Index;
        struct player* playerBad;

        if(oppTeam == 'A') 
        {
            playerBad_Index = getPlayerBadIndex(&(Ref->time), &(Ref->teamB));
            playerBad = & Ref->teamB.members[playerBad_Index];
        } 
        else if (oppTeam == 'B') 
        {
            playerBad_Index = getPlayerBadIndex(&(Ref->time) ,&(Ref->teamA));
            playerBad = & Ref->teamA.members[playerBad_Index];
        } 
        else 
        {
            printf("Errore nella penalità.\n");
        }

        int penMins = getPenaltyTime();
        playerBad->resumePlay = (Ref->time) + penMins; 
        getPenaltyMSG(msg, playerBad, penMins);
    }
    
    void addDribbling(struct stats* stats) { stats -> numberDribbling++; }
    void addShotFailed(struct stats* stats) { stats -> shotFailed++; }

#endif