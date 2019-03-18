#define _GNU_SOURCE
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/mman.h>

#include "shmSpecs.h"

Shm *shm;                   // pointer to shared memory

sem_t *sleeping = SEM_FAILED;
sem_t *arrive = SEM_FAILED;
sem_t *queue = SEM_FAILED;
sem_t *invite = SEM_FAILED;
sem_t *ready = SEM_FAILED;
sem_t *shaving = SEM_FAILED;


void close_all(){
    if(shm != (void*) -1){
        if(munmap(shm,sizeof(Shm)) < 0) printf("Error while unmapping shared memory!\n");
    }
    if(sleeping != SEM_FAILED){
        if (sem_close(sleeping) < 0) printf("Error while closing semaphores!\n");
    }
    if(arrive != SEM_FAILED){
        if (sem_close(arrive) < 0) printf("Error while closing semaphores!\n");
    }
    if(queue != SEM_FAILED){
        if (sem_close(queue) < 0) printf("Error while closing semaphores!\n");
    }
    if(invite != SEM_FAILED){
        if (sem_close(invite) < 0) printf("Error while closing semaphores!\n");
    }
    if(ready != SEM_FAILED){
        if (sem_close(ready) < 0) printf("Error while closing semaphores!\n");
    }
    if(shaving != SEM_FAILED){
        if (sem_close(shaving) < 0) printf("Error while closing semaphores!\n");
    }
}

void handleSIGINT(int signo){
    printf("\nReceived SIGINT.\n");
    exit(0);
}

double get_timestamp(){
    struct timespec stamp;
    clock_gettime(CLOCK_MONOTONIC,&stamp);
    return (double)stamp.tv_sec + 1.0e-9*stamp.tv_nsec;
}

void handler(int signo){
    printf("[%f] Client %d: Called from queue by the barber.\n",get_timestamp(),getpid());
}

void open_semaphores(){
    sleeping = sem_open(SLEEPING_NAME,O_RDWR);
    arrive = sem_open(ARRIVE_NAME, O_RDWR);
    queue = sem_open(QUEUE_NAME, O_RDWR);
    invite = sem_open(INVITE_NAME, O_RDWR);
    ready = sem_open(READY_NAME, O_RDWR);
    shaving = sem_open(SHAVING_NAME, O_RDWR);
    if(sleeping == SEM_FAILED || arrive == SEM_FAILED || queue == SEM_FAILED || invite == SEM_FAILED || ready == SEM_FAILED || shaving == SEM_FAILED){
        printf("Error while opening semaphores!\n");
        exit(EXIT_FAILURE);
    }
}

void block_semaphore(sem_t *sem){
    sem_wait(sem);
}

void unblock_semaphore(sem_t *sem){
    sem_post(sem);
}

//________________________________MAIN_____________________________________

int main(int argc, char **argv){

    atexit(close_all);

    signal(SIGINT, handleSIGINT);

    if(argc != 3){
        printf("Pass number of clients and shavings!\n");
        return EXIT_FAILURE;
    }

    int clients = (int) strtol(argv[1],NULL,10);
    int shavings = (int) strtol(argv[2],NULL,10);


    if(clients < 1 || shavings < 1){
        printf("Arguments must be positive.\n");
        return EXIT_FAILURE;
    }
    
    char* path = getenv("HOME");
    if(path == NULL){
        printf("Couldnt get $HOME value!\n");
        return EXIT_FAILURE;
    }
    
    int shmid = shm_open(SHM_NAME, O_RDWR, DEFFILEMODE);
    if(shmid < 0){
        printf("Couldnt open shared memory segment!\n");
        return EXIT_FAILURE;
    }

    shm = (Shm*) mmap(NULL, sizeof(Shm), PROT_READ|PROT_WRITE, MAP_SHARED, shmid, 0);
    if(shm < 0){
        printf("Couldnt get a pointer to the shared memory!\n");
        return EXIT_FAILURE;
    }

    open_semaphores();

    
    for(int i=0; i<clients; i++){
        int child = fork();
        if(child == 0){
            int pid = getpid();
            for(int j=0; j<shavings; j++){
                block_semaphore(arrive);                // can I check on barber
                if(shm->barber_asleep == 1){                // barber is sleeping
                    shm->current_client = pid;              // I'm first
                    printf("[%f] Client %d: Waking up barber.\n",get_timestamp(),pid);
                    unblock_semaphore(sleeping);                   // waking up barber
                    block_semaphore(invite);                    // wait until barber lets me sit
                    printf("[%f] Client %d: Sitting on a chair.\n",get_timestamp(),pid);
                    unblock_semaphore(ready);                   // sitting on a chair
                    block_semaphore(shaving);                   // waiting for barber to finish shaving
                    printf("[%f] Client %d: Leaving after being shaved.\n",get_timestamp(),pid);
                }
                
                else{                                   // barber is working
                    block_semaphore(queue);                 // check the waiting room
                    if(shm->clients_sitting == shm->seats){ // no place
                        printf("[%f] Client %d: No place in the waiting room. Leaving.\n",get_timestamp(),pid);
                        unblock_semaphore(queue);               // allow others to check waiting room
                        unblock_semaphore(arrive);              // another person can enter 
                    }
                    else{                                   // getting in line
                        shm->fifo[shm->clients_sitting] = getpid(); // taking a seat
                        shm->clients_sitting++;                     // counting clients in the waiting room
                        printf("[%f] Client %d: Getting in line. Position: %d.\n",get_timestamp(),pid,shm->clients_sitting);
                        unblock_semaphore(queue);                   // allow others to check waiting room
                        unblock_semaphore(arrive);                  // another person can enter 
                        sigset_t mask;
                        sigfillset(&mask);
                        sigdelset(&mask,SIGUSR1);
                        signal(SIGUSR1,handler);
                        sigsuspend(&mask);                          // wait for barber to call me
                        printf("[%f] Client %d: Sitting on a chair.\n",get_timestamp(),pid);
                        unblock_semaphore(ready);                   // sitting on a chair
                        block_semaphore(shaving);                   // waiting for barber to finish shaving
                        printf("[%f] Client %d: Leaving after being shaved.\n",get_timestamp(),pid);
                    }
                }
            }
            return EXIT_SUCCESS;
        }
    }

    
    while(wait(NULL) > 0);          // waiting for all children

    return EXIT_SUCCESS;
}