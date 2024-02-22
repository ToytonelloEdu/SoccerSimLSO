#ifndef GLSTRUCT
#define GLSTRUCT

#include <pthread.h>
typedef struct referee;
typedef struct team;
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

struct team //struct modeling a football team of 5 players
{
    int membersFD[5]; //file descriptors of sockets for each player/client
    pthread_t membersTID[5]; //id for each thread managing of one the players/clients


};

struct referee //struct modeling the referee of the game
{
    struct team teamA; 
    struct team teamB;
    int logFD; //file descriptor of the game log file

};

#endif