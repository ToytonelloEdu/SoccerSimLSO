#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "gameLogicFuncts.h"
#include "gameLogicStructs.h"

int main(int argc, char* argv[]){

    struct referee ref;

    initTeam(&(ref.teamB));
    ref.teamB.teamName = "Real Madrid";
    initPlayer(ref.teamB.captain, "Ronaldo", 7, &(ref.teamB.teamName));
    
    /* pid_t pid = fork();
    if(pid == 0){
        initPlayer(&(ref.teamB.members[1]), "Benzema", 9, &(ref.teamB.teamName));

        pid_t pid2 = fork();
        if(pid2 == 0){
            initPlayer(&(ref.teamB.members[2]), "Bale", 11, &(ref.teamB.teamName));
        }
        
        pid_t pid3 = fork();
        if(pid3 == 0){
            initPlayer(&(ref.teamB.members[3]), "Isco", 15, &(ref.teamB.teamName));
        } else {
            initPlayer(&(ref.teamB.members[4]), "Modric", 10, &(ref.teamB.teamName));
        }
    } */

    int fd;
    struct sockaddr_un indirizzo;

    indirizzo.sun_family = AF_LOCAL;
    strcpy(indirizzo.sun_path, "/tmp/mio_socket");

    fd = socket(PF_LOCAL, SOCK_STREAM, 0);

    int result = connect(fd, (struct sockaddr*)&indirizzo, sizeof(indirizzo));
    if(result == -1){
        perror("client");
        exit(1);
    }

    char writebuffer[100] = "Antonio gay";
    int lung2 = strlen(writebuffer);
    write(fd, writebuffer, lung2);

    char readbuffer[100] = "";
    read(fd, readbuffer, 100);
    int lung = strlen(readbuffer);
    for(int i = 0; i < lung; i++){
        printf("%c", readbuffer[i]);
    }

    close(fd);

}