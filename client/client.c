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
    printf("Welcome to LSOccer Simulator's Client!\n");

    int fd;
    struct sockaddr_un address;

    address.sun_family = AF_INET;
    strcpy(address.sun_path, "127.0.0.1");

    fd = socket(PF_INET, SOCK_STREAM, 0);
    
    int result = connect(fd, (struct sockaddr*)&address, sizeof(address));
    if(result == -1)
    {
        perror("client");
        exit(1);
    }

    char buffer[BUFFSIZE] = ""; 
    int rdout = read(fd, buffer, BUFFSIZE);
    printf("%s\n", buffer);

    close(fd);

    return 0;
}