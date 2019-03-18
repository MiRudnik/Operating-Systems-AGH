#define _GNU_SOURCE
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "shmSpecs.h"

Shm *shm;                   // pointer to shared memory
int semid = -1;             // semaphore group identifier


double get_timestamp(){
    struct timespec stamp;
    clock_gettime(CLOCK_MONOTONIC,&stamp);
    return (double)stamp.tv_sec + 1.0e-9*stamp.tv_nsec;
}

void handler(int signo){
    printf("[%f] Client %d: Called from queue by the barber.\n",get_timestamp(),getpid());
}

void block_semaphore(int sem){
    struct sembuf operation;
    operation.sem_num = sem;
    operation.sem_op = -1;
    operation.sem_flg = 0;
    semop(semid,&operation,1);
}

void unblock_semaphore(int sem){
    struct sembuf operation;
    operation.sem_num = sem;
    operation.sem_op = 1;
    operation.sem_flg = 0;
    semop(semid,&operation,1);
}

//________________________________MAIN_____________________________________

int main(int argc, char **argv){

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
    
    key_t shmkey = ftok(path,PROJECTID);
    
    int shmid = shmget(shmkey,sizeof(Shm),0666);
    if(shmid < 0){
        printf("Couldnt get a shared memory segment! Is barber shop open?\n");
        return EXIT_FAILURE;
    }

    shm = (Shm*) shmat(shmid,NULL,0);
    if(shm < 0){
        printf("Couldnt get a pointer to the shared memory!\n");
        return EXIT_FAILURE;
    }

    key_t semkey = ftok("./", PROJECTID);
    
    semid = semget(semkey, 0, 0666);
    if(semid < 0){
        printf("Couldnt get semaphores!\n");
        return EXIT_FAILURE;
    }

    
    for(int i=0; i<clients; i++){
        int child = fork();
        if(child == 0){
            int pid = getpid();
            for(int j=0; j<shavings; j++){
                block_semaphore(ARRIVE);                // can I check on barber
                if(shm->barber_asleep == 1){                // barber is sleeping
                    shm->current_client = pid;              // I'm first
                    printf("[%f] Client %d: Waking up barber.\n",get_timestamp(),pid);
                    unblock_semaphore(SLEEP);                   // waking up barber
                    block_semaphore(INVITE);                    // wait until barber lets me sit
                    printf("[%f] Client %d: Sitting on a chair.\n",get_timestamp(),pid);
                    unblock_semaphore(READY);                   // sitting on a chair
                    block_semaphore(SHAVING);                   // waiting for barber to finish shaving
                    printf("[%f] Client %d: Leaving after being shaved.\n",get_timestamp(),pid);
                }
                
                else{                                   // barber is working
                    block_semaphore(QUEUE);                 // check the waiting room
                    if(shm->clients_sitting == shm->seats){ // no place
                        printf("[%f] Client %d: No place in the waiting room. Leaving.\n",get_timestamp(),pid);
                        unblock_semaphore(QUEUE);               // allow others to check waiting room
                        unblock_semaphore(ARRIVE);              // another person can enter 
                    }
                    else{                                   // getting in line
                        shm->fifo[shm->clients_sitting] = getpid(); // taking a seat
                        shm->clients_sitting++;                     // counting clients in the waiting room
                        printf("[%f] Client %d: Getting in line. Position: %d.\n",get_timestamp(),pid,shm->clients_sitting);
                        unblock_semaphore(QUEUE);                   // allow others to check waiting room
                        unblock_semaphore(ARRIVE);                  // another person can enter 
                        sigset_t mask;
                        sigfillset(&mask);
                        sigdelset(&mask,SIGUSR1);
                        signal(SIGUSR1,handler);
                        sigsuspend(&mask);                          // wait for barber to call me
                        printf("[%f] Client %d: Sitting on a chair.\n",get_timestamp(),pid);
                        unblock_semaphore(READY);                   // sitting on a chair
                        block_semaphore(SHAVING);                   // waiting for barber to finish shaving
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