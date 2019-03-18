#ifndef shmSpecs_h
#define shmSpecs_h

#define PROJECTID 7
#define MAXSEATS 20

// semaphore numbers
#define SLEEP 0
#define ARRIVE 1 
#define QUEUE 2
#define INVITE 3
#define READY 4
#define SHAVING 5


typedef struct Shm{
    int seats;
    int barber_asleep;
    int clients_sitting;
    int current_client;
    int fifo[MAXSEATS];
} Shm;

#endif