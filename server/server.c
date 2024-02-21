#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#define BUFFSIZE 512

int main()
{
    printf("Welcome to LSOccer Simulator's Server!\n");

    int fd1, new_socket;
    struct sockaddr_un address;

    address.sun_family = AF_LOCAL;
    strcpy(address.sun_path, "/tmp/mySocket");

    fd1 = socket(PF_LOCAL, SOCK_STREAM, 0);
    bind(fd1, (struct sockaddr*)&address, sizeof(address));

    listen(fd1, 5);

    //char buffer[BUFFSIZE] = "";

    while((new_socket = accept(fd1, NULL, NULL)) > -1)
    {
        char buffer[] = "Cirogay";
        int wrout = write(new_socket, buffer, strlen(buffer));
        printf("Messaggio mandato\n");
        close(new_socket);
    }

    close (fd1);
    unlink("/tmp/mySocket");

    return 0;
}