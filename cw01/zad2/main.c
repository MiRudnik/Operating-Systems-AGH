#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#include <string.h>
#include <unistd.h>

#include "blockLibrary.h"

struct fullTime{
    __clock_t timeR;
    __clock_t timeU;
    __clock_t timeS;
};

struct fullTime mkTimeStamp(){
    struct fullTime timeStamp;
    struct timespec time_r; // tv_sec - full secs, tv_nsec - nsecs
    clock_gettime(CLOCK_REALTIME, &time_r);

    timeStamp.timeR = time_r.tv_nsec + time_r.tv_sec * 1000000000;

    struct tms time_s_u; // tms_stime tms_utime
    times(&time_s_u);

    timeStamp.timeS = time_s_u.tms_stime;
    timeStamp.timeU = time_s_u.tms_utime;

    return  timeStamp;
}

char* generateBlock(int maxLength){
    char* allowedSings = "abcdefghijklmnopqrstuvwxyz1234567890";
    int signs = strlen(allowedSings);
    int randChar;
    int blockLength = 1 + rand()%(maxLength); // no empty blocks
    char* block = (char*) malloc(blockLength * sizeof(char));
    for(int i=0;i<blockLength-1;i++){
        randChar = rand()%signs;
        block[i] = allowedSings[randChar];
    }
    block[blockLength-1] = '\0';
    return block;
}

void fillArray_S(struct arrayS* array){
    for(int i=0;i<array->size;i++){
        char* block = generateBlock(array->length);
        addBlock_S(array,block,i);
    }
}

void fillArray_D(struct arrayD* array){
    for(int i=0;i<array->size;i++){
        char* block = generateBlock(array->length);
        addBlock_D(array,block,i);
    }
}

void add_S(struct arrayS* array, int number){
    for(int i=0;i<number;i++){
        int index = rand()%(array->size);
        char* block = generateBlock(array->length);
        addBlock_S(array,block,index);
    }
}

void add_D(struct arrayD* array, int number){
    for(int i=0;i<number;i++){
        int index = rand()%(array->size);
        char* block = generateBlock(array->length);
        addBlock_D(array,block,index);
    }
}

void remove_S(struct arrayS* array, int number){
    for(int i=0;i<number;i++){
        int index = rand()%(array->size);
        deleteBlock_S(array,index);
    }
}

void remove_D(struct arrayD* array, int number){
    for(int i=0;i<number;i++){
        int index = rand()%(array->size);
        deleteBlock_D(array,index);
    }
}

void removeAndAdd_S(struct arrayS* array, int number){
    for(int i=0;i<number;i++){
        int index = rand()%(array->size);
        deleteBlock_S(array,index);
        char* block = generateBlock(array->length);
        addBlock_S(array,block,index);
    }
}

void removeAndAdd_D(struct arrayD* array, int number){
    for(int i=0;i<number;i++){
        int index = rand()%(array->size);
        deleteBlock_D(array,index);
        char* block = generateBlock(array->length);
        addBlock_D(array,block,index);
    }
}

int main(int argc, char **argv){
    // argv[0] - program call
    srand(time(NULL));
    bool isStatic;
    if(strcmp(argv[1],"static") == 0){
        isStatic = 1;

    }
    else if(strcmp(argv[1],"dynamic") == 0){
        isStatic = 0;
    }
    else{
        printf("Pass in the first argument 'static' or 'dynamic'\n");
        return 1;
    }

    char* pEnd;
    int size = (int) strtol(argv[2], &pEnd, 10);
    int length = (int) strtol(argv[3], &pEnd, 10);

    if(size <= 0 || length <= 0 || size > MAXSIZE || length > MAXSIZE){
        printf("Incorrect size or/and length. Max is 10000\n");
        return 1;
    }

    int argCounter = 4;
    int number = 0;

    if(isStatic){
        struct fullTime makeStamp1 = mkTimeStamp();

        struct arrayS* myArray = createBlockArray_S(size,length);
        fillArray_S(myArray);

        struct fullTime makeStamp2 = mkTimeStamp();
        printf("#### StaticArray ####\n***** Make Time: *****\nReal time: %f\nUser time: %f\nSystem time: %f\n\n",
               (float)((makeStamp2.timeR-makeStamp1.timeR)/1000000000.0),
               (float)(makeStamp2.timeU-makeStamp1.timeU)/(float)sysconf(_SC_CLK_TCK),
               (float)(makeStamp2.timeS-makeStamp1.timeS)/(float)sysconf(_SC_CLK_TCK));

        while(argCounter < argc){
            if(strcmp(argv[argCounter],"search") == 0){
                number = (int) strtol(argv[argCounter+1], &pEnd, 10);
                if(number == 0){
                    printf("Wrong arguments\n");
                    return 1;
                }
                else{
                    struct fullTime searchStamp1 = mkTimeStamp();

                    int result = searchClosestAscii_S(myArray,number);
                    if(result == -1) printf("Wrong index or there is only one block in the array.\n\n");
                    else printf("Block at index %d is closest in value to block at index %d.\n\n",result,number);

                    struct fullTime searchStamp2 = mkTimeStamp();
                    printf("***** Search Time: *****\nReal time: %f\nUser time: %f\nSystem time: %f\n\n",
                           (float)((searchStamp2.timeR-searchStamp1.timeR)/1000000000.0),
                           (float)(searchStamp2.timeU-searchStamp1.timeU)/(float)sysconf(_SC_CLK_TCK),
                           (float)(searchStamp2.timeS-searchStamp1.timeS)/(float)sysconf(_SC_CLK_TCK));
                }
            }
            else if(strcmp(argv[argCounter],"add") == 0){
                number = (int) strtol(argv[argCounter+1], &pEnd, 10);
                if(number == 0){
                    printf("Wrong arguments\n");
                    return 1;
                }
                else{
                    struct fullTime addStamp1 = mkTimeStamp();

                    add_S(myArray,number);

                    struct fullTime addStamp2 = mkTimeStamp();
                    printf("***** Add Time: *****\nReal time: %f\nUser time: %f\nSystem time: %f\n\n",
                           (float)((addStamp2.timeR-addStamp1.timeR)/1000000000.0),
                           (float)(addStamp2.timeU-addStamp1.timeU)/(float)sysconf(_SC_CLK_TCK),
                           (float)(addStamp2.timeS-addStamp1.timeS)/(float)sysconf(_SC_CLK_TCK));
                }
            }
            else if(strcmp(argv[argCounter],"remove") == 0){
                number = (int) strtol(argv[argCounter+1], &pEnd, 10);
                if(number == 0){
                    printf("Wrong arguments\n");
                    return 1;
                }
                else{
                    struct fullTime removeStamp1 = mkTimeStamp();

                    remove_S(myArray,number);

                    struct fullTime removeStamp2 = mkTimeStamp();
                    printf("***** Remove Time: *****\nReal time: %f\nUser time: %f\nSystem time: %f\n\n",
                           (float)((removeStamp2.timeR-removeStamp1.timeR)/1000000000.0),
                           (float)(removeStamp2.timeU-removeStamp1.timeU)/(float)sysconf(_SC_CLK_TCK),
                           (float)(removeStamp2.timeS-removeStamp1.timeS)/(float)sysconf(_SC_CLK_TCK));
                }
            }
            else if(strcmp(argv[argCounter],"remove_and_add") == 0){
                number = (int) strtol(argv[argCounter+1], &pEnd, 10);
                if(number == 0){
                    printf("Wrong arguments\n");
                    return 1;
                }
                else{
                    struct fullTime remAddStamp1 = mkTimeStamp();

                    removeAndAdd_S(myArray,number);

                    struct fullTime remAddStamp2 = mkTimeStamp();
                    printf("***** Remove and Add Time: *****\nReal time: %f\nUser time: %f\nSystem time: %f\n\n\n",
                           (float)((remAddStamp2.timeR-remAddStamp1.timeR)/1000000000.0),
                           (float)(remAddStamp2.timeU-remAddStamp1.timeU)/(float)sysconf(_SC_CLK_TCK),
                           (float)(remAddStamp2.timeS-remAddStamp1.timeS)/(float)sysconf(_SC_CLK_TCK));
                }
            }
            else {
                printf("Wrong arguments\n");
                return 1;
            }
            argCounter = argCounter + 2;
        }

    }
    else{
        struct fullTime makeStamp1 = mkTimeStamp();

        struct arrayD* myArray = createBlockArray_D(size,length);
        fillArray_D(myArray);

        struct fullTime makeStamp2 = mkTimeStamp();
        printf("#### Dynamic Array ####\n***** Make Time: *****\nReal time: %f\nUser time: %f\nSystem time: %f\n\n",
               (float)((makeStamp2.timeR-makeStamp1.timeR)/1000000000.0),
               (float)(makeStamp2.timeU-makeStamp1.timeU)/(float)sysconf(_SC_CLK_TCK),
               (float)(makeStamp2.timeS-makeStamp1.timeS)/(float)sysconf(_SC_CLK_TCK));

        while(argCounter < argc){
            if(strcmp(argv[argCounter],"search") == 0){
                number = (int) strtol(argv[argCounter+1], &pEnd, 10);
                if(number == 0){
                    printf("Wrong arguments\n");
                    return 1;
                }
                else{
                    struct fullTime searchStamp1 = mkTimeStamp();

                    int result = searchClosestAscii_D(myArray,number);
                    if(result == -1) printf("Wrong index or there is only one block in the array.\n\n");
                    else printf("Block at index %d is closest in value to block at index %d.\n\n",result,number);

                    struct fullTime searchStamp2 = mkTimeStamp();
                    printf("***** Search Time: *****\nReal time: %f\nUser time: %f\nSystem time: %f\n\n",
                           (float)((searchStamp2.timeR-searchStamp1.timeR)/1000000000.0),
                           (float)(searchStamp2.timeU-searchStamp1.timeU)/(float)sysconf(_SC_CLK_TCK),
                           (float)(searchStamp2.timeS-searchStamp1.timeS)/(float)sysconf(_SC_CLK_TCK));

                }
            }
            else if(strcmp(argv[argCounter],"add") == 0){
                number = (int) strtol(argv[argCounter+1], &pEnd, 10);
                if(number == 0){
                    printf("Wrong arguments\n");
                    return 1;
                }
                else{
                    struct fullTime addStamp1 = mkTimeStamp();

                    add_D(myArray,number);

                    struct fullTime addStamp2 = mkTimeStamp();
                    printf("***** Add Time: *****\nReal time: %f\nUser time: %f\nSystem time: %f\n\n",
                           (float)((addStamp2.timeR-addStamp1.timeR)/1000000000.0),
                           (float)(addStamp2.timeU-addStamp1.timeU)/(float)sysconf(_SC_CLK_TCK),
                           (float)(addStamp2.timeS-addStamp1.timeS)/(float)sysconf(_SC_CLK_TCK));
                }
            }
            else if(strcmp(argv[argCounter],"remove") == 0){
                number = (int) strtol(argv[argCounter+1], &pEnd, 10);
                if(number == 0){
                    printf("Wrong arguments\n");
                    return 1;
                }
                else{
                    struct fullTime removeStamp1 = mkTimeStamp();

                    remove_D(myArray,number);

                    struct fullTime removeStamp2 = mkTimeStamp();
                    printf("***** Remove Time: *****\nReal time: %f\nUser time: %f\nSystem time: %f\n\n",
                           (float)((removeStamp2.timeR-removeStamp1.timeR)/1000000000.0),
                           (float)(removeStamp2.timeU-removeStamp1.timeU)/(float)sysconf(_SC_CLK_TCK),
                           (float)(removeStamp2.timeS-removeStamp1.timeS)/(float)sysconf(_SC_CLK_TCK));
                }
            }
            else if(strcmp(argv[argCounter],"remove_and_add") == 0){
                number = (int) strtol(argv[argCounter+1], &pEnd, 10);
                if(number == 0){
                    printf("Wrong arguments\n");
                    return 1;
                }
                else{
                    struct fullTime remAddStamp1 = mkTimeStamp();

                    removeAndAdd_D(myArray,number);

                    struct fullTime remAddStamp2 = mkTimeStamp();
                    printf("***** Remove and Add Time: *****\nReal time: %f\nUser time: %f\nSystem time: %f\n\n\n",
                           (float)((remAddStamp2.timeR-remAddStamp1.timeR)/1000000000.0),
                           (float)(remAddStamp2.timeU-remAddStamp1.timeU)/(float)sysconf(_SC_CLK_TCK),
                           (float)(remAddStamp2.timeS-remAddStamp1.timeS)/(float)sysconf(_SC_CLK_TCK));
                }
            }
            else {
                printf("Wrong arguments\n");
                return 1;
            }
            argCounter = argCounter + 2;
        }
    }



    return 0;
}
