
#ifndef GLSTRUCT
#define GLSTRUCT

#include <pthread.h>
typedef struct referee;
typedef struct team;
typedef struct player;
typedef struct stats;
typedef int ball;

    void wait(ball mutex)
    {
        while(mutex <= 0);
        mutex--;
    }

    void signal(ball mutex)
    {
        mutex++;
    }

    struct player
    {
        int playerFD;
        char* name;
        char shirtNumber; 
        pthread_t playerTID;           //id for each thread managing of one the players/clients
    };

    struct team                        //struct modeling a football team of 5 players
    {
        char* teamName;
        struct player members[5];      //file descriptors of sockets for each player/client
        struct player* captain;         
    };

    void initTeam(struct team team)
    {
        team.captain = &(team.members[0]);
    }

    struct stats                       //struct modeling the stats of the game
    {
        int numberGoal;
        int shotFailed;
        int numberDribbling;
    };    

    struct referee                     //struct modeling the referee of the game
    {
        struct team teamA; 
        struct team teamB;
        struct stats stats;
        int logFD;                     //file descriptor of the game log file
        int time;
    };

    void goal(struct referee* referee, struct player* player)
    {
        referee -> stats.numberGoal++;
        int FD = referee -> logFD;


    }
    
    void addDribbling(struct stats* stats) { stats -> numberDribbling++; }
    void addShotFailed(struct stats* stats) { stats -> shotFailed++; }

#endif