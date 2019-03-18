#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

int gotSIGTSTP = 0;

void handleSIGINT(int signo){
    printf("\nReceived signal SIGINT\n");
    exit(EXIT_SUCCESS);
}

void handleSIGTSTP(int signo){
    if(signo == SIGTSTP){
        if(gotSIGTSTP == 0){
            gotSIGTSTP = 1;
            printf("\nWaiting for CTRL+Z - continue, or CTRL+C - exit program\n");
        }
        else{
            gotSIGTSTP = 0;
        }
    }
    else{
        printf("\nWaiting for CTRL+Z - continue, or CTRL+C - exit program\n");
    }
}


int main(void){
    struct sigaction actions;
    actions.sa_handler = handleSIGTSTP;
    sigemptyset(&actions.sa_mask);
    actions.sa_flags = 0;
    sigaction(SIGTSTP, &actions, NULL);
    
    signal(SIGINT,handleSIGINT);
    
    while(1){
        time_t currentTime = time(NULL);
        printf("%s",ctime(&currentTime)); // converts time_t into string
        sleep(1);
        if (gotSIGTSTP == 1) pause();
    }
    return 0;
}