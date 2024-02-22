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

ball S = 1;

void foo()
{
    wait(S);

    //random selection:
    //-shoot
    //-dribbling
    //-infortuns

    signal(S);
}

int main()
{
    struct referee Ref;
    Ref.teamA.membersFD[0] = 1;
    Ref.teamB.membersFD[0] = 2;
    return 0;
}