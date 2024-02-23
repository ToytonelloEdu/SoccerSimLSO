#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "gameLogicFuncts.h"
#include "gameLogicStructs.h"

void *gestisci(void *arg, void* player){
    char readbuffer[100] = "";
    read ((int *) arg, readbuffer, 100);
    int lung = strlen(readbuffer);
    for(int i = 0; i < lung; i++){
        printf("%c", readbuffer[i]);
    }
    printf("\n");

    char writebuffer[100] = "Fortunato down\n";
    int lung2 = strlen(writebuffer);
    write((int *) arg, writebuffer, lung2);
    printPlayer((struct player*) player);
    //close((int *) arg);
    pthread_exit(0);
}

int main(void){

    struct referee ref;

    int fd1, new_socket;
    struct sockaddr_un indirizzo;
    pthread_t tid;

    indirizzo.sun_family = AF_LOCAL;
    strcpy(indirizzo.sun_path, "/tmp/mio_socket");

    fd1 = socket(AF_LOCAL, SOCK_STREAM, 0);
    bind(fd1, (struct sockaddr*)&indirizzo, sizeof(indirizzo));

    listen(fd1, 5);

    for(int i = 0; i < 1; i++){

        ref.teamB.members[i].playerFD = accept(fd1, NULL, NULL);
        //printPlayer(&(ref.teamB.captain));

        printf("%d\n", ref.teamB.members[i].playerFD);
        pthread_create(&(ref.teamB.members[i].playerTID), NULL, gestisci, ((void *) ref.teamB.members[i].playerFD, (void *) ref.teamB.captain));
        pthread_detach(ref.teamB.members[i].playerTID);

        }


    close(fd1);
    unlink("/tmp/mio_socket");

}