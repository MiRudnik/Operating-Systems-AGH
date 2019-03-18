#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>

#define EQUALS 0
#define LESSER -1
#define GREATER 1

#define SPECIFIC 1
#define BRIEF 0

int P;
int K;
int N;
int L;
FILE* file;
int op;   // (-1 - '<', 0 - '=', 1 - '>')
int type; // 0-brief, 1-specific
int nk;

char **buffer;
pthread_t *threads;

int producerPosition = 0;
int consumerPosition = 0;

sem_t *buffer_semaphores;
sem_t array_sem;
sem_t file_sem;
sem_t full_sem;
sem_t empty_sem;

void finish(){
    fclose(file);
    free(buffer);
    free(buffer_semaphores);
}

void handle_exit(int signo){
    
    if(signo == SIGALRM) printf("Time has elapsed!\n");
    exit(EXIT_SUCCESS);
}

bool validate_length(int length){
    switch (op){
        case -1:
            if(length < L) return true;
            break;
        case 0:
            if(length == L) return true;
            break;
        case 1:
            if(length > L) return true;
            break;
    }
    return false;
}

void *produce(void *arg){
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL); // ?
    int id = *((int*) arg);
    char line[512];
    int pointer;

    while(1){

        sem_wait(&empty_sem);       // wait for consumer to get element
        
        sem_wait(&array_sem);
        pointer = producerPosition;
        producerPosition = (producerPosition+1) % N;
        sem_wait(&buffer_semaphores[pointer]);
        sem_post(&array_sem);

        sem_wait(&file_sem);
        if(fgets(line, 512, file) == NULL){
            printf("End of file.\n");
            exit(EXIT_SUCCESS);
        }
        sem_post(&file_sem);
        buffer[pointer] = malloc((strlen(line)+1) * sizeof(char));
        strcpy(buffer[pointer],line);
        if(type == SPECIFIC) printf("[Producer %d] Put line at position %d.\n",id,pointer);
        sem_post(&full_sem);     // inform that element has been added

        sem_post(&buffer_semaphores[pointer]);
    }

}

void *consume(void *arg){
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL); // ?
    int id = *((int*) arg);
    int length;
    char line[512];
    int pointer;
    
    while(1){
        
        sem_wait(&full_sem);        // // wait for producer to provide element

        sem_wait(&array_sem);
        pointer = consumerPosition;
        consumerPosition = (consumerPosition+1) % N;
        sem_wait(&buffer_semaphores[pointer]);
        sem_post(&array_sem);

        strcpy(line,buffer[pointer]);
        free(buffer[pointer]);
        buffer[pointer] = NULL;

        length = strlen(line);
        if(validate_length(length) == true) printf("[Consumer %d] Index %d: %s\n",id,pointer,line);
        else if(type == SPECIFIC) printf("[Consumer %d] Index %d: Incorrect length.\n",id,pointer);
        sem_post(&empty_sem);            // inform that element has been taken
        
        sem_post(&buffer_semaphores[pointer]);
    }
}

void get_parameters(char* filename){

    FILE *cfg = fopen(filename, "r");
    if(cfg == NULL){
        printf("Couldn't open file %s!\n",filename);
        exit(EXIT_FAILURE);
    }

    char buff[80];
    char *arg;
    if(fgets(buff, 80, cfg) == NULL){
        printf("Empty file!\n");
        exit(EXIT_FAILURE);
    }
    fclose(cfg);
    printf("%s\n",buff);
    
    for(int i=0; i<8; i++){
        if((arg = strtok((i == 0 ? buff : NULL), " \t\n\r")) == NULL){
            printf("Incorrect config format!\n");
            exit(EXIT_FAILURE);
        }
        
        switch (i){
            case 0:
                P = (int) strtol(arg,NULL,10);
                if(P < 1){
                    printf("Incorrect config format! (P)\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 1:
                K = (int) strtol(arg,NULL,10);
                if(K < 1){
                    printf("Incorrect config format! (K)\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 2:
                N = (int) strtol(arg,NULL,10);
                if(N < 1){
                    printf("Incorrect config format! (N)\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 3:
                file = fopen(arg, "r");
                if(file == NULL){
                    printf("Couldn't open file %s!\n",arg);
                    exit(EXIT_FAILURE);
                }
                break;
            case 4:
                L = (int) strtol(arg,NULL,10);
                if(L < 1){
                    printf("Incorrect config format! (L)\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 5:
                op = (int) strtol(arg,NULL,10);
                if(op < -1 || op > 1){
                    printf("Incorrect config format! (op)\n%d",op);
                    exit(EXIT_FAILURE);
                }
                break;
            case 6:
                type = (int) strtol(arg,NULL,10);
                if(type != 0 && type != 1){
                    printf("Incorrect config format! (type)\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 7:
                nk = (int) strtol(arg,NULL,10);
                if(nk < 0){
                    printf("Incorrect config format! (nk)\n");
                    exit(EXIT_FAILURE);
                }
                break;
        }
    }
}

int main(int argc, char **argv){

    if(argc != 2){
        printf("Give config file as an argument!\n");
        return EXIT_FAILURE;
    }

    get_parameters(argv[1]);
    printf("%d %d %d %d %d %d %d\n",P,K,N,L,op,type,nk); // TEST

    buffer = calloc(N, sizeof(char *));
    buffer_semaphores = calloc(N, sizeof(sem_t));

    for (int i = 0; i < N; ++i) {
        sem_init(&buffer_semaphores[i], 0, 1);
    }
    sem_init(&array_sem, 0, 1);
    sem_init(&file_sem, 0, 1);
    sem_init(&empty_sem, 0, N);    // counting places for elements in buffer
    sem_init(&full_sem, 0, 0);     // counting elements in buffer

    threads = calloc(P+K, sizeof(pthread_t));
    int ids[P+K];

    atexit(finish);

    for(int i=0;i<P;i++){           // CREATING THREADS
        ids[i] = i;
        pthread_create(&threads[i], NULL, &produce, (void *)&ids[i]);
    }
    for(int i=P;i<P+K;i++){
        ids[i] = i;
        pthread_create(&threads[i], NULL, &consume, (void *)&ids[i]);
    }
    signal(SIGINT, &handle_exit);
    signal(SIGALRM, &handle_exit);

    if(nk > 0){
        alarm(nk);
    }
    
    for(int i=P;i<P+K;i++){         // WAITING FOR THREADS
        pthread_join(threads[i], NULL);
    }

    return EXIT_SUCCESS;
}