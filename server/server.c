#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

int main()
{
    printf("Welcome to LSOccer Simulator's Server!");

    int fd1, new_socket;
    struct sockaddr_un address;

    address.sun_family = AF_LOCAL;
    strcpy(address.sun_path, "/tmp/mySocket");

    fd1 = socket(PF_LOCAL, SOCK_STREAM, 0);
    bind(fd1, (struct sockaddr*)&address, sizeof(address));

    listen(fd1, 5);

    while((new_socket = accept(fd1, NULL, NULL)) > -1)
    {
        //azioni del server
        printf("Siamo gay");
    }

    close (fd1);
    unlink("/tmp/MySocket");

    return 0;
}