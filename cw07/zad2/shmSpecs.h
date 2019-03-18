#ifndef shmSpecs_h
#define shmSpecs_h

#define PROJECTID 7
#define MAXSEATS 20

const char *SHM_NAME = "/shared";

// semaphore names
const char *SLEEPING_NAME = "/sleeping_sem";
const char *ARRIVE_NAME = "/arrive_sem";
const char *QUEUE_NAME = "/queue_sem";
const char *INVITE_NAME = "/invite_sem";
const char *READY_NAME = "/ready_sem";
const char *SHAVING_NAME = "/shaving_sem";

typedef struct Shm{
    int seats;
    int barber_asleep;
    int clients_sitting;
    int current_client;
    int fifo[MAXSEATS];
} Shm;

#endif