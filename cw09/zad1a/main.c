#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

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
pthread_mutex_t *buffer_mutexes;
pthread_t *threads;

int producerPosition = 0;
int consumerPosition = 0;
int elements = 0;

pthread_mutex_t array_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t full = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;

void finish(){
    fclose(file);
    free(buffer);
    free(buffer_mutexes);
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

        pthread_mutex_lock(&array_mutex);
        
        while(elements == N) {          // if full
            pthread_cond_wait(&full, &array_mutex);
        }
        pointer = producerPosition;
        elements++;
        producerPosition = (producerPosition+1) % N;
        pthread_mutex_lock(&buffer_mutexes[pointer]);
        pthread_mutex_unlock(&array_mutex);
        
        pthread_mutex_lock(&file_mutex);
        if(fgets(line, 512, file) == NULL){
            printf("End of file.\n");
            exit(EXIT_SUCCESS);
        }
        pthread_mutex_unlock(&file_mutex);
        buffer[pointer] = malloc((strlen(line)+1) * sizeof(char));
        strcpy(buffer[pointer],line);
        if(type == SPECIFIC) printf("[Producer %d] Put line at position %d.\n",id,pointer);
        pthread_cond_broadcast(&empty);     // inform that buffor is not empty
        pthread_mutex_unlock(&buffer_mutexes[pointer]);

    }

}

void *consume(void *arg){
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL); // ?
    int id = *((int*) arg);
    int length;
    char line[512];
    int pointer;
    
    while(1){
        
        pthread_mutex_lock(&array_mutex);
        while(elements == 0) {          // if empty
            pthread_cond_wait(&empty, &array_mutex);
        }
        pointer = consumerPosition;
        elements--;
        consumerPosition = (consumerPosition+1) % N;
        
        pthread_mutex_lock(&buffer_mutexes[pointer]);
        pthread_mutex_unlock(&array_mutex);

        strcpy(line,buffer[pointer]);
        free(buffer[pointer]);
        buffer[pointer] = NULL;

        length = strlen(line);
        if(validate_length(length) == true) printf("[Consumer %d] Index %d: %s\n",id,pointer,line);
        else if(type == SPECIFIC) printf("[Consumer %d] Index %d: Incorrect length.\n",id,pointer);
        

        pthread_cond_broadcast(&full);     // inform that buffor is not full
        pthread_mutex_unlock(&buffer_mutexes[pointer]);

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

    buffer = calloc(N, sizeof(char *));
    buffer_mutexes = calloc(N, sizeof(pthread_mutex_t));

    for (int i = 0; i < N; ++i) {
        pthread_mutex_init(&buffer_mutexes[i], NULL);
    }

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