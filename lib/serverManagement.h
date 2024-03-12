#ifndef SRVMNG
#define SRVMNG

#include <time.h>
#include <stdio.h>
#include <fcntl.h>

int createNewLogFile()
{
    char path[100], buffer[26];
    time_t hour;
    time(&hour);
    ctime_r(&hour, buffer);
    sprintf(path, "~/SoccerSimLSO/server/log/%s-gameLog.txt", buffer);
    return creat(path, O_RDWR);
}






 #endif
