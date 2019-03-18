#define _GNU_SOURCE // for sa_sigaction
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <bits/siginfo.h>
#include <wait.h>

int N;
int M;
int remaining;
int requests = 0;
pid_t* processes;
pid_t* waiting;

void handleSIGINT(int signo, siginfo_t *info, void *context){
    int killed = 0;
    printf("\nReceived signal SIGINT\n");
    for(int i=0; i<N; i++){
        if(processes[i] > 0){
            kill(processes[i], SIGKILL);
            killed++;
        }
    }
    printf("Killed %d processes, exiting program.\n",killed);
    exit(EXIT_SUCCESS);
}

void handleSIGCHLD(int signo, siginfo_t *info, void *context){  // when child ends his job
    printf("Process %d returned with value %d.\n",info->si_pid,info->si_status);
    if (remaining == 0)
        {
            printf("All processes ended. Exiting program.\n");
            free(waiting);
            free(processes);
            exit(EXIT_SUCCESS);
    }
}

void handleRequest(int signo, siginfo_t *info, void *context){
printf("Received signal SIGUSR1 from process %d.\n", info->si_pid);
    if (requests < M) {
        waiting[requests] = info->si_pid;
        requests++;
        printf("Request queued. Currently: %d, needed: %d\n", requests, M);
        if (requests >= M) {    // if currest request surpassed needed number
            printf("Enough requests. Answering...\n");
            for (int i = 0; i < requests; ++i) {
                if (waiting[i] > 0) {
                    printf("Sending signal SIGUSR1 to process %d.\n", waiting[i]);
                    kill(waiting[i], SIGUSR1);
                    waitpid(waiting[i], NULL, 0);
                    remaining--;
                }
            }
        }
    } else {
        printf("Sending signal SIGUSR1 to process %d.\n",
               info->si_pid);
        kill(info->si_pid, SIGUSR1);
        waitpid(info->si_pid, NULL, 0);
        remaining--;
    }
}

void handleRTSig(int signo, siginfo_t *info, void *context){
    printf("Received signal SIGRTMIN+%d from process %d.\n", signo-SIGRTMIN, info->si_pid);
}

int main(int argc, char** argv){
    if(argc != 3){
        printf("Pass 2 arguments!\n");
        exit(EXIT_FAILURE);
    }
    N = (int) strtol(argv[1], NULL, 10);
    M = (int) strtol(argv[2], NULL, 10);
    if(N < 1 || M < 1 || N < M){
        printf("0 < M <= N\n");
        exit(EXIT_FAILURE);
    }

    processes = malloc(N * sizeof(int));
    waiting = malloc(M * sizeof(int));
    
    remaining = N;

    struct sigaction actions;
    sigemptyset(&actions.sa_mask);
    actions.sa_flags = SA_NOCLDSTOP | SA_SIGINFO;
    // SA_NOCLDSTOP - dont generate SIGCHLD when children stop
    // SA_SIGINFO - additional info about process (requires different function template)
    
    actions.sa_sigaction = handleSIGINT;
    sigaction(SIGINT, &actions, NULL);
    
    actions.sa_sigaction = handleSIGCHLD;
    sigaction(SIGCHLD, &actions, NULL);

    actions.sa_sigaction = handleRequest;
    sigaction(SIGUSR1, &actions, NULL);

    actions.sa_sigaction = handleRTSig;
    for(int i=SIGRTMIN; i<=SIGRTMAX; i++){
        sigaction(i, &actions, NULL);
    }

    for(int i=0; i<N; i++){
        sleep(1);
        pid_t pid = fork();
        if (pid < 0) {
            printf("Error while creating child process.\n");
            exit(EXIT_FAILURE);
        }
        
        if(pid){    // parent
            processes[i] = pid;
        }
        else{       // child
            execl("./doChild","./doChild",NULL);
        }

    }

    while(wait(NULL));  // waiting for ALL children to be killed

    exit(EXIT_SUCCESS);
}