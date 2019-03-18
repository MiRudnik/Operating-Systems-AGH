#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

int pid;            // id of child process
int running = 1;    // is program printing date

void handleSIGINT(int signo){
    printf("\nReceived signal SIGINT\n");
    kill(pid, SIGKILL);
    printf("Killing child process, exiting program.\n");
    exit(EXIT_SUCCESS);
}

void handleSIGTSTP(int signo){
    if(running == 1){
        running = 0;
        if(kill(pid, SIGKILL) == 0) printf("\nKilling child process. \n");
            else printf("\nError on killing child process!\n");
    }
    else{
        running = 1;
        printf("\nUnpausing program\n");
    }
}


int main(void){
    int status;
    struct sigaction actions;
    actions.sa_handler = handleSIGTSTP;
    sigemptyset(&actions.sa_mask);
    actions.sa_flags = 0;
    sigaction(SIGTSTP, &actions, NULL);

    signal(SIGINT,handleSIGINT);
    
    while(1){
        if(running == 1){
            pid = (int) fork();
            if(!pid){
                execlp("./printDate.sh","./printDate.sh",NULL);
            }
            else {
                waitpid(pid, &status, 0);
            }
        }
    }
    return 0;
}