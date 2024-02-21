#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]){

    fork();
    
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