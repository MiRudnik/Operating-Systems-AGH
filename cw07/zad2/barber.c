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

int shmid = -1;             // shared memory identifier
Shm *shm;                   // pointer to shared memory

sem_t *sleeping = SEM_FAILED;
sem_t *arrive = SEM_FAILED;
sem_t *queue = SEM_FAILED;
sem_t *invite = SEM_FAILED;
sem_t *ready = SEM_FAILED;
sem_t *shaving = SEM_FAILED;


void handleSIGTERM(int signo){
    printf("Received SIGTERM.\nClosing barber shop. Come back tomorrow!\n");
    exit(2);
}

void cleanup(){
    if(shmid != -1){
        if(munmap(shm,sizeof(Shm)) < 0) printf("Error while unmapping shared memory!\n");
        if (shm_unlink(SHM_NAME) < 0) printf("Error while removing shared memory!\n");
    }
    if(sleeping != SEM_FAILED){
        if (sem_close(sleeping) < 0) printf("Error while closing semaphores!\n");
        if (sem_unlink(SLEEPING_NAME) < 0) printf("Error while removing semaphores!\n");
    }
    if(arrive != SEM_FAILED){
        if (sem_close(arrive) < 0) printf("Error while closing semaphores!\n");
        if (sem_unlink(ARRIVE_NAME) < 0) printf("Error while removing semaphores!\n");
    }
    if(queue != SEM_FAILED){
        if (sem_close(queue) < 0) printf("Error while closing semaphores!\n");
        if (sem_unlink(QUEUE_NAME) < 0) printf("Error while removing semaphores!\n");
    }
    if(invite != SEM_FAILED){
        if (sem_close(invite) < 0) printf("Error while closing semaphores!\n");
        if (sem_unlink(INVITE_NAME) < 0) printf("Error while removing semaphores!\n");
    }
    if(ready != SEM_FAILED){
        if (sem_close(ready) < 0) printf("Error while closing semaphores!\n");
        if (sem_unlink(READY_NAME) < 0) printf("Error while removing semaphores!\n");
    }
    if(shaving != SEM_FAILED){
        if (sem_close(shaving) < 0) printf("Error while closing semaphores!\n");
        if (sem_unlink(SHAVING_NAME) < 0) printf("Error while removing semaphores!\n");
    }
}

double get_timestamp(){
    struct timespec stamp;
    clock_gettime(CLOCK_MONOTONIC,&stamp);
    return (double)stamp.tv_sec + 1.0e-9*stamp.tv_nsec;
}

void set_semaphores(){
    sleeping = sem_open(SLEEPING_NAME, O_CREAT|O_EXCL, 0666, 0);
    arrive = sem_open(ARRIVE_NAME, O_CREAT|O_EXCL, 0666, 1);
    queue = sem_open(QUEUE_NAME, O_CREAT|O_EXCL, 0666, 1);
    invite = sem_open(INVITE_NAME, O_CREAT|O_EXCL, 0666, 0);
    ready = sem_open(READY_NAME, O_CREAT|O_EXCL, 0666, 0);
    shaving = sem_open(SHAVING_NAME, O_CREAT|O_EXCL, 0666, 0);
    if(sleeping == SEM_FAILED || arrive == SEM_FAILED || queue == SEM_FAILED || invite == SEM_FAILED || ready == SEM_FAILED || shaving == SEM_FAILED){
        printf("Error while making semaphores!\n");
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

    atexit(cleanup);
    signal(SIGTERM,handleSIGTERM);
    
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask,SIGINT);
    
    sigprocmask(SIG_BLOCK,&mask,NULL);      // blocking SIGINT

    if(argc != 2){
        printf("Pass number of seats as only argument!\n");
        return EXIT_FAILURE;
    }

    int seats = (int) strtol(argv[1],NULL,10);

    if(seats > MAXSEATS || seats < 1){
        printf("Wrong number of seats! (Maximum is %d)\n",MAXSEATS);
        return EXIT_FAILURE;
    }
    
    char* path = getenv("HOME");
    if(path == NULL){
        printf("Couldnt get $HOME value!\n");
        return EXIT_FAILURE;
    }
     
    shmid = shm_open(SHM_NAME, O_CREAT|O_EXCL|O_RDWR, DEFFILEMODE);
    if(shmid < 0){
        printf("Couldnt create shared memory segment!\n");
        return EXIT_FAILURE;
    }

    if(ftruncate(shmid, sizeof(Shm)) < 0){
        printf("Couldnt set the size of shared memory segment!\n");
        return EXIT_FAILURE;
    }

    shm = (Shm*) mmap(NULL, sizeof(Shm), PROT_READ|PROT_WRITE, MAP_SHARED, shmid, 0);
    if(shm < 0){
        printf("Couldnt get a pointer to the shared memory!\n");
        return EXIT_FAILURE;
    }

    shm->clients_sitting = 0;
    shm->seats = seats;
    shm->barber_asleep = 0;

    set_semaphores();
        // sleeping 0, arrive 1, queue 1, invite 0, ready 0, shaving 0 - starting values

    printf("Opening barber shop with %d seats.\n",seats);

    while(1){
        block_semaphore(arrive);        // block arriving, now I'm looking for clients
        block_semaphore(queue);         // I need to check for clients 
        if(shm->clients_sitting == 0){      // no clients waiting
            unblock_semaphore(queue);                           // clients can queue now
            printf("[%f] Barber: Falling asleep.\n",get_timestamp());
            shm->barber_asleep = 1;                             // going to sleep
            unblock_semaphore(arrive);                          // let client enter and wake me up
            block_semaphore(sleeping);                             // waiting for client to wake me up
            printf("[%f] Barber: Waking up.\n",get_timestamp());
            shm->barber_asleep = 0;                             // client woke me up
            unblock_semaphore(arrive);                          // let others check on me since I'm starting working
            printf("[%f] Barber: Inviting client %d for shaving.\n",get_timestamp(),shm->current_client);
            unblock_semaphore(invite);                          // invite client to the chair
            block_semaphore(ready);                             // wait for client to sit
            printf("[%f] Barber: Starting shaving client %d.\n",get_timestamp(),shm->current_client);
            printf("[%f] Barber: Finishing shaving client %d.\n",get_timestamp(),shm->current_client);
            unblock_semaphore(shaving);                         // I finished shaving, client can leave
        }
        else{                               // clients waiting
            shm->current_client = shm->fifo[0];                 // getting current client and moving queue
            for(int i=0; i<shm->clients_sitting-1; i++) shm->fifo[i] = shm->fifo[i+1]; 
            shm->clients_sitting--;
            printf("[%f] Barber: Inviting client %d for shaving.\n",get_timestamp(),shm->current_client);
            unblock_semaphore(queue);                           // clients can queue now
            unblock_semaphore(arrive);                          // let others check on me since I'm starting working
            kill(shm->current_client,SIGUSR1);
            block_semaphore(ready);                             // wait for client to sit
            printf("[%f] Barber: Starting shaving client %d.\n",get_timestamp(),shm->current_client);
            printf("[%f] Barber: Finishing shaving client %d.\n",get_timestamp(),shm->current_client);
            unblock_semaphore(shaving);                         // I finished shaving, client can leave
        }

    }

    return EXIT_SUCCESS;
}