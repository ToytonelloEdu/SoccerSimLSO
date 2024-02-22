
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
struct referee Ref;
enum actions {shot, injury, dribbling};

    void selectAction(struct player* player)
    { 
        wait(S);
        sleep(2);

        int result = rand() % 3;
        
        if (result == "shot" ) { shot(player); }
        if (result == "injury" ) { injury(player); }
        if (result == "dribbling" ) { dribbling(player); }

        signal(S);
    }

    void shot(struct player* player)
    {
        int result = rand() % 2;    //0 GOAL -- 1 NO GOAL

        if (result == 0) 
        { goal(&Ref, &player); } 
        else
        { addShotFailed(&(Ref.stats)); }
    }

    void injury(struct player* player)
    {

    }

    void dribbling(struct player* player)
    {
        int result = rand() % 2;    //0 SUCCESS -- 1 FAILED

        addDribbling(&(Ref.stats));

        if (result == 0) { shot(&player); }
    }

    int main()
    {
        return 0;
    }