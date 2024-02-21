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
    printf("Welcome to LSOccer Simulator's Server!\n");

    int wsock_fd, new_socket;
    int opt = 1;
    struct sockaddr_in servaddr, cliaddr;
    
    if((wsock_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }
    if(setsockopt(wsock_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(12345);

    if(bind(wsock_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if(listen(wsock_fd, 10) < 0)
    {
        perror("listen error");
        exit(EXIT_FAILURE);
    }

    int addrlen = sizeof(cliaddr);

    while((new_socket = accept(wsock_fd, (struct sockaddr*)&cliaddr, (socklen_t*)&addrlen)) > -1)
    {
        char buffer[] = "Cirogay";
        int wrout = write(new_socket, buffer, strlen(buffer));
        printf("Messaggio mandato\n");
        close(new_socket);
    }

    close (wsock_fd);

    return 0;
}