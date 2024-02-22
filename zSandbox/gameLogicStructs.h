
#ifndef GLSTRUCT
#define GLSTRUCT

#include <pthread.h>
struct referee;
struct team;
struct player;
struct stats;

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

#endif