#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <wait.h>

int child;
int Type;
int receivedParent = 0;
int receivedChild = 0;
int wasSIGINT = 0;

void parentSignal1(int signo, siginfo_t *info, void *context){
    receivedParent++;
    if(Type != 3) printf("Received %d signal SIGUSR1 from child.\n", receivedParent);
    else printf("Received %d signal SIGRTMIN from child.\n", receivedParent);
}

void parentSIGINT(int signo, siginfo_t *info, void *context){
    wasSIGINT = 1;
    printf("\nReceived SIGINT. Stoping child and exiting program...\n");
    if(Type != 3) kill(child, SIGUSR2);
    else kill(child, SIGRTMAX);
}

void childSignal1(int signo, siginfo_t *info, void *context){
    receivedChild++;
    if(Type != 3){
        printf("Received %d signal SIGUSR1 from parent. Sending back...\n", receivedChild);
        kill(getppid(), SIGUSR1);
    }
    else{
        printf("Received %d signal SIGRTMIN from parent. Sending back...\n", receivedChild);
        kill(getppid(), SIGRTMIN); 
    }
}

void childSignal2(int signo, siginfo_t *info, void *context){
    if(wasSIGINT == 0){
        if(Type != 3) printf("Received signal SIGUSR2 from parent. Finishing...\n");
        else printf("Received signal SIGRTMAX from parent. Finishing...\n");
        printf("Child received %d signals.\n", receivedChild);
    }
    exit(EXIT_SUCCESS);
}

int main(int argc, char** argv){
    if(argc != 3){
        printf("Pass 2 arguments!\n");
        exit(EXIT_FAILURE);
    }
    int L = (int) strtol(argv[1], NULL, 10);
    Type = (int) strtol(argv[2], NULL, 10);
    if(L < 1 || Type < 1 || Type > 3){
        printf("L > 0, Type from 1 to 3!\n");
        exit(EXIT_FAILURE);
    }

    struct sigaction actions;
    sigemptyset(&actions.sa_mask);
    actions.sa_flags = SA_NOCLDSTOP | SA_SIGINFO;

    child = (int) fork();
    
    if(!child){  // parent
        sleep(1);

        if(Type != 3){
            
            actions.sa_sigaction = parentSIGINT;
            sigaction(SIGINT, &actions, NULL);

            actions.sa_sigaction = parentSignal1;
            sigaction(SIGUSR1, &actions, NULL);

            sigset_t newmask;       // if Type == 2
            sigfillset(&newmask);
            sigdelset(&newmask, SIGUSR1);
            sigdelset(&newmask, SIGINT);    // waiting also for SIGINT
            
            for(int i = 0; i < L; i++) {
                printf("Sending %d signal SIGUSR1 to child.\n", i+1);
                kill(child, SIGUSR1);
                if(Type == 2) sigsuspend(&newmask); // waiting for response
            }
            printf("Sending SIGUSR2 to child.\n");
            kill(child, SIGUSR2);
        }
        else{
            
            actions.sa_sigaction = parentSignal1;
            sigaction(SIGRTMIN, &actions, NULL);

            for(int i = 0; i < L; i++) {
                printf("Sending %d signal SIGRTMIN to child.\n", i+1);
                kill(child, SIGRTMIN);
            }
            printf("Sending SIGRTMAX to child.\n");
            kill(child, SIGRTMAX);
        }
    }
    else{       // child
        sigset_t newmask;
        
        if(Type != 3){  
            actions.sa_sigaction = childSignal1;
            sigaction(SIGUSR1, &actions, NULL);

            actions.sa_sigaction = childSignal2;
            sigaction(SIGUSR2, &actions, NULL);

            sigfillset(&newmask);
            sigdelset(&newmask, SIGUSR1);
            sigdelset(&newmask, SIGUSR2);
        }
        else{
            actions.sa_sigaction = childSignal1;
            sigaction(SIGRTMIN, &actions, NULL);

            actions.sa_sigaction = childSignal2;
            sigaction(SIGRTMAX, &actions, NULL);

            sigfillset(&newmask);
            sigdelset(&newmask, SIGRTMIN);
            sigdelset(&newmask, SIGRTMAX);
        }
        sigprocmask(SIG_SETMASK, &newmask, NULL);
        while (1);  // do nothing
    }
    int status;
    waitpid(child, &status, 0);
    
    printf("Parent received %d signals.\n", receivedParent);
    exit(EXIT_SUCCESS);
}