#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

void *gestisci(void *arg){
    char readbuffer[100] = "";
    read ((int *) arg, readbuffer, 100);
    int lung = strlen(readbuffer);
    for(int i = 0; i < lung; i++){
        printf("%c", readbuffer[i]);
    }
    printf("\n");

    char writebuffer[100] = "Fortunato down";
    int lung2 = strlen(writebuffer);
    write((int *) arg, writebuffer, lung2);
    //close((int *) arg);
    pthread_exit(0);
}

int main(void){

    int fd1, new_socket;
    struct sockaddr_un indirizzo;
    pthread_t tid;

    indirizzo.sun_family = AF_LOCAL;
    strcpy(indirizzo.sun_path, "/tmp/mio_socket");

    fd1 = socket(AF_LOCAL, SOCK_STREAM, 0);
    bind(fd1, (struct sockaddr*)&indirizzo, sizeof(indirizzo));

    listen(fd1, 5);

    while((new_socket = accept(fd1, NULL, NULL)) > -1){

        printf("%d\n", new_socket);
        pthread_create(&tid, NULL, gestisci, (void *) new_socket);
        pthread_detach(tid);

    }

    close(fd1);
    unlink("/tmp/mio_socket");

}