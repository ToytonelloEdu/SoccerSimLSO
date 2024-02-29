
#ifndef GLSTRUCT
#define GLSTRUCT

#include <pthread.h>
#include <string.h>

enum gameStatusEnum {nogame, gameCreated, waitingOtherCaptain , gameCreation , oneCaptainNeeded};

struct referee;
struct team;
struct player;
struct stats;

    struct player //struct modeling players/clients
    {
        int playerFD;                   //file descriptor of socket of the player/client
        char* name;                     
        char shirtNumber; 
        char* teamName;
        pthread_t playerTID;           //id for thread managing of the player/client
    };

    void initPlayer(struct player* player, char* name, char shirtNum, char* team)
    {
        player -> name = name;
        player -> shirtNumber = shirtNum;
        player -> teamName = team;
    }

    void printPlayer(struct player* player)
    {
        printf("\t(No %d) %s for team %s\n", player->shirtNumber, player->name, player->teamName);
    }
    

    struct team                        //struct modeling a football team of 5 players
    {
        char teamName[15];                
        struct player members[5];      
        struct player* captain;  
        char membNum;      
    };

    void initTeam(struct team* team)
    {
        team->captain = &(team->members[0]);
        team->membNum = 0;
        for(int i = 0; i < 5; i++)
            team->members[i].teamName = team->teamName;
    }

    struct stats                       //struct modeling the stats of the game
    {
        int numberGoal;
        int shotFailed;
        int numberDribbling;
    };    

    struct referee                     //struct modeling the referee of the game
    {
        enum gameStatusEnum gameStatus;
        pthread_t lastThread;

        struct team teamA; 
        struct team teamB;
        struct stats stats;
        int logFD;                     //file descriptor of the game log file
        int time;
    };

    void InitReferee(struct referee* referee)
    {
        referee->gameStatus = nogame;
        referee->time = -1;
        
    }

#endif