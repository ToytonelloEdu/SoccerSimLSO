
#ifndef GLSTRUCT
#define GLSTRUCT

#include <stdio.h>
#include <pthread.h>
#include <string.h>

#define TEAMSIZE 5
#define BUFFSIZE 512
#define NAMESIZE 30
#define PATHSIZE 54

enum gameStatusEnum {nogame, gameCreated, waitingOtherCaptain, gameCreation, oneCaptainNeeded, gameRestarting, gameStarting, gameStarted, gameFinished, gameDisbanded};

struct referee;
struct team;
struct player;
struct stats;

    struct player //struct modeling players/clients
    {
        int FD;                   //file descriptor of socket of the player/client
        char name[NAMESIZE];                     
        char shirtNumber; 
        char team;
        char* teamName;
        pthread_t TID;           //id for thread managing of the player/client
        int resumePlay;
    };

    void initPlayer(struct player* player, char* name, char shirtNum)
    {
        strncpy(player->name, name, NAMESIZE);
        player -> shirtNumber = shirtNum;
        player -> resumePlay = 0;
    }

    void setPlayerTeam(struct player* player, char team, char* teamName)
    {
        player->team = team;
        player->teamName = teamName;
    }

    void setPlayerSysVar(struct player* player, int FD, pthread_t TID)
    {
        player->FD = FD;
        player->TID = TID;
    }

    void copyPlayer(struct player* dest, struct player src)
    {
        strncpy(dest->name, src.name, NAMESIZE);
        dest->shirtNumber = src.shirtNumber;
        dest->team = src.team;
        dest->teamName = src.teamName;
        dest->FD = src.FD;
        dest->TID = src.TID;
        dest->resumePlay = src.resumePlay;
    }

    void printPlayer(struct player* player)
    {
        printf("\t(No %d) %s for team %s\n", player->shirtNumber, player->name, player->teamName);
    }

    void PlayerToString(char* buffer, struct player player)
    {
        char tmp[100] = "";
        sprintf(tmp, " \t(No %d) %s for team %s\n", player.shirtNumber, player.name, player.teamName);
        //if(strlen(tmp) <= sizeof(buffer))
        strncpy(buffer, tmp, strlen(tmp));
    }
    

    struct team                        //struct modeling a football team of 5 players
    {
        char teamName[NAMESIZE];                
        struct player members[TEAMSIZE];      
        struct player* captain;  
        char membNum;      
    };

    void initTeam(struct team* team)
    {
        team->captain = &(team->members[0]);
        team->membNum = 0;
    }

    struct stats                       //struct modeling the stats of the game
    {
        int numberGoalA;
        int numberGoalB;
        int shotFailed;
        int numberDribbling;
    };    

    void initStats(struct stats* stats)
    {
        stats->numberGoalA = 0;
        stats->numberGoalB = 0;
        stats->shotFailed = 0;
        stats->numberDribbling = 0;
    }

    struct referee                     //struct modeling the referee of the game
    {
        enum gameStatusEnum gameStatus;
        pthread_t lastThread;
        pthread_t clockThread;

        struct team teamA; 
        struct team teamB;
        struct stats stats;
        int logFD;                    //file descriptor of the game log file
        char pathLogServer[PATHSIZE];
        int time;
    };

    void InitReferee(struct referee* referee)
    {
        referee->gameStatus = nogame;
        referee->time = -1;
        initStats(& referee->stats);

    }

    void ResetRef(struct referee* referee)
    {
        for(int i = 0; i < TEAMSIZE; i++)
        {
            referee->teamA.members[i].resumePlay = 0;
            referee->teamB.members[i].resumePlay = 0;
        }
        referee->gameStatus = gameRestarting;
        referee->time = -1;
        initStats(& referee->stats);
    }

#endif