#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

void handleSIGUSR1(int signo){
    kill(getppid(), rand()%32 + SIGRTMIN);
}

int main(){
    srand((unsigned int) getpid());
    sigset_t newmask;
    sigfillset(&newmask);
    sigdelset(&newmask, SIGUSR1);   // only SIGUSR1 unblocked
    
    signal(SIGUSR1, handleSIGUSR1);

    int howLong = rand()%11;
    printf("Process %d sleeping for %d secs.\n",(int)getpid(),howLong);
    sleep(howLong);
    kill(getppid(), SIGUSR1);
    sigsuspend(&newmask);
    return howLong;
}