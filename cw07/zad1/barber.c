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

int shmid = -1;             // shared memory identifier
Shm *shm;                   // pointer to shared memory
int semid = -1;             // semaphore group identifier


void handleSIGTERM(int signo){
    printf("Received SIGTERM.\nClosing barber shop. Come back tomorrow!\n");
    exit(2);
}

void cleanup(){
    if(shmid != -1){
        shmdt(shm);
        if (shmctl(shmid, IPC_RMID, NULL) < 0) printf("Error while removing shared memory!\n");
    }
    if(semid != -1){
        if (semctl(semid, 0, IPC_RMID, NULL) < 0) printf("Error while removing semaphores!\n");
    }
}

double get_timestamp(){
    struct timespec stamp;
    clock_gettime(CLOCK_MONOTONIC,&stamp);
    return (double)stamp.tv_sec + 1.0e-9*stamp.tv_nsec;
}

void set_semaphores(){
    semctl(semid,SLEEP,SETVAL,0);
    semctl(semid,ARRIVE,SETVAL,1);
    semctl(semid,QUEUE,SETVAL,1);
    semctl(semid,INVITE,SETVAL,0);
    semctl(semid,READY,SETVAL,0);
    semctl(semid,SHAVING,SETVAL,0);
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
    
    key_t shmkey = ftok(path,PROJECTID);
    
    shmid = shmget(shmkey,sizeof(Shm),IPC_CREAT | IPC_EXCL | 0666);
    if(shmid < 0){
        printf("Couldnt create shared memory segment!\n");
        return EXIT_FAILURE;
    }

    shm = (Shm*) shmat(shmid,NULL,0);
    if(shm < 0){
        printf("Couldnt get a pointer to the shared memory!\n");
        return EXIT_FAILURE;
    }

    shm->clients_sitting = 0;
    shm->seats = seats;
    shm->barber_asleep = 0;

    key_t semkey = ftok("./", PROJECTID);
    
    semid = semget(semkey, 6, IPC_CREAT | IPC_EXCL | 0666);
    if(semid < 0){
        printf("Couldnt create semaphores!\n");
        return EXIT_FAILURE;
    }

    set_semaphores();
        // SLEEP 0, ARRIVE 1, QUEUE 1, INVITE 0, READY 0, SHAVING 0 - starting values

    printf("Opening barber shop with %d seats.\n",seats);

    while(1){
        block_semaphore(ARRIVE);        // block arriving, now I'm looking for clients
        block_semaphore(QUEUE);         // I need to check for clients 
        if(shm->clients_sitting == 0){      // no clients waiting
            unblock_semaphore(QUEUE);                           // clients can queue now
            printf("[%f] Barber: Falling asleep.\n",get_timestamp());
            shm->barber_asleep = 1;                             // going to sleep
            unblock_semaphore(ARRIVE);                          // let client enter and wake me up
            block_semaphore(SLEEP);                             // waiting for client to wake me up
            printf("[%f] Barber: Waking up.\n",get_timestamp());
            shm->barber_asleep = 0;                             // client woke me up
            unblock_semaphore(ARRIVE);                          // let others check on me since I'm starting working
            printf("[%f] Barber: Inviting client %d for shaving.\n",get_timestamp(),shm->current_client);
            unblock_semaphore(INVITE);                          // invite client to the chair
            block_semaphore(READY);                             // wait for client to sit
            printf("[%f] Barber: Starting shaving client %d.\n",get_timestamp(),shm->current_client);
            printf("[%f] Barber: Finishing shaving client %d.\n",get_timestamp(),shm->current_client);
            unblock_semaphore(SHAVING);                         // I finished shaving, client can leave
        }
        else{                               // clients waiting
            shm->current_client = shm->fifo[0];                 // getting current client and moving queue
            for(int i=0; i<shm->clients_sitting-1; i++) shm->fifo[i] = shm->fifo[i+1]; 
            shm->clients_sitting--;
            printf("[%f] Barber: Inviting client %d for shaving.\n",get_timestamp(),shm->current_client);
            unblock_semaphore(QUEUE);                           // clients can queue now
            unblock_semaphore(ARRIVE);                          // let others check on me since I'm starting working
            kill(shm->current_client,SIGUSR1);
            block_semaphore(READY);                             // wait for client to sit
            printf("[%f] Barber: Starting shaving client %d.\n",get_timestamp(),shm->current_client);
            printf("[%f] Barber: Finishing shaving client %d.\n",get_timestamp(),shm->current_client);
            unblock_semaphore(SHAVING);                         // I finished shaving, client can leave
        }

    }

    return EXIT_SUCCESS;
}