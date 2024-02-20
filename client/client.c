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
    printf("Welcome to LSOccer Simulator's Client!");

    int fd;
    struct sockaddr_un address;

    address.sun_family = AF_LOCAL;
    strcpy(address.sun_path, "/tmp/MySocket");

    fd = socket(PF_LOCAL, SOCK_STREAM, 0);
    
    int result = connect(fd, (struct sockaddr*)&address, sizeof(address));
    if(result == -1)
    {
        perror("client");
        exit(1);
    }

    //azioni client

    close(fd);

    return 0;
}