#include "internal/log/qulog.h"
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>


void log_request(int status, char request_info[], char timediff[]) {
    time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    char ftime[80];
    strftime(ftime,80,"%Y/%m/%d - %H:%M:%S", timeinfo);

    if (status == 200) {
        // time - status - handling time | origin IP | method | path
        printf("[QUINTESSENCE] %s |\033[1;37;42m %d \033[0m| %s | %s", ftime, status, timediff, request_info); // highlight green, reset
    } else {
        printf("[QUINTESSENCE] %s |\033[1;37;41m %d \033[0m| %s | %s", ftime, status, timediff, request_info); // highlight red, reset
    }
}