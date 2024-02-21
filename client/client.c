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

#define BUFFSIZE 512

int main()
{
    printf("Welcome to LSOccer Simulator's Client!\n");

    int sock_fd, status;
    struct sockaddr_in servaddr;

    if((sock_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(12345);

    int n;
    
    if(status = connect(sock_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0);
    {
        perror("client");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFSIZE] = ""; 
    int rdout = read(sock_fd, buffer, BUFFSIZE);
    printf("%s\n", buffer);

    close(sock_fd);

    return 0;
}