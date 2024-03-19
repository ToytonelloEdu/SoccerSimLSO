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

char* names[10] = {"Antonio", "Ciro", "Fortunato", "Matteo", "Luigi", "Donato", "Vincenzo", "Alessandro", "Francesco", "Arturo"};
int numbers[10] = {35, 7, 9, 3, 11, 10, 24, 14, 18, 99};

int main(int argc, char* argv[])
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
    servaddr.sin_port = htons(12345);
    if(argc == 2) { servaddr.sin_addr.s_addr = inet_addr(argv[1]); }
    else { perror("Address"); exit(1); }
    

    int n;
    
    if(status = connect(sock_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("client");
        exit(EXIT_FAILURE);
    }

    int rdout = 1;
    do
    {
        char rBuffer[BUFFSIZE] = ""; 
        rdout = read(sock_fd, rBuffer, BUFFSIZE);
        
        for(int i = 3; i < strlen(rBuffer); i++)
        {
            printf("%c", rBuffer[i]);
        }

        switch (rBuffer[1])
        {
        case '0':
            write(sock_fd, "", 1);
            break;
        case '1':
            char wBuffer[BUFFSIZE];
            fgets(wBuffer, BUFFSIZE, stdin);
            write(sock_fd, wBuffer, strlen(wBuffer));
            break;
        case '2': NULL;
            break;
        case '3':
            write(sock_fd, "CLI_ERROR", 10);
            break;
        case '9':
            rdout = -1;
            break;
        
        default: write(sock_fd, "SRV_ERROR", 10);
            break;
        }
        
    }while(rdout > -1);

    close(sock_fd);

    return 0;
}