#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

struct fullTime{
    __clock_t timeU;
    __clock_t timeS;
};

struct fullTime mkTimeStamp(){
    struct fullTime timeStamp;

    struct tms time_s_u; // tms_stime tms_utime
    times(&time_s_u);

    timeStamp.timeS = time_s_u.tms_stime;
    timeStamp.timeU = time_s_u.tms_utime;

    return  timeStamp;
}

void generate(char* fileName, int number, int size){
    int fileDesc = open(fileName,O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
    char* record = malloc(size * sizeof(char));
        
    for(int i=0;i<number;i++){
        for(int j=0;j<size;j++){
            record[j] = rand()%93 + 33;
        }
        if(write(fileDesc, record, size) != size){
            printf("Writing failed!\n");
            return;
        }
    }
    free(record);
    close(fileDesc);
}

void copy_sys(char* from, char* to, int number, int size){
    int src = open(from, O_RDONLY);
    int dest = open(to, O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR);
    
    char* buf = malloc(size * sizeof(char));
    for(int i=0;i<number;i++){
        if(read(src, buf, size) != size){
            printf("Error while reading file!\n");
            return;
        }
        if(write(dest, buf, size) != size){
            printf("Error while writing to file!\n");
            return;
        }
    }
    close(dest);
    close(src);
}

void sort_sys(char* fileName, int number, int size){
    int fileDesc = open(fileName, O_RDWR);
    char* sortRecord = malloc(size * sizeof(char));         // element being sorted
    char* cmpRecord = malloc(size * sizeof(char));          // element to compare to 
    int sortKey, cmpKey;
    long int offset = (long int) size * sizeof(char);

    for(int i=1;i<number;i++){
        lseek(fileDesc, offset * i, SEEK_SET);              // picking element (skip first) 
        if(read(fileDesc, sortRecord, size) != size){
            printf("Error while reading file!\n");
            return;
        }

        lseek(fileDesc, offset * (-2), SEEK_CUR);           // element before one being sorted atm
        if(read(fileDesc, cmpRecord, size) != size){
            printf("Error while reading file!\n");
            return;
        }
        
        sortKey = (int) sortRecord[0];
        cmpKey = (int) cmpRecord[0];

        int c = i;                                          // counter how many element before the sorted one

        while(sortKey < cmpKey && c != 1){
            
            if(write(fileDesc, cmpRecord, size) != size){   // moving elements to the right
                printf("Error while writing to file!\n");
                return;
            }

            lseek(fileDesc, offset * (-3), SEEK_CUR);       // back one element
            if(read(fileDesc, cmpRecord, size) != size){
                printf("Error while reading file!\n");
                return;
            }
            cmpKey = (int) cmpRecord[0];
            c--;                                            // counting until we cant move back more
        }

        if(sortKey < cmpKey){                               // if the first element is greater than sorted one
            if(write(fileDesc, cmpRecord, size) != size){   // write it on the second place
                printf("Error while writing to file!\n");
                return;
            }
            lseek(fileDesc, offset * (-2), SEEK_CUR);       // move the pointer to write sorted element on the first place
        }
        
        if(write(fileDesc, sortRecord, size) != size){      // writing sorted elememt
            printf("Error while writing to file!\n");
            return;
        }
        
    }
    free(sortRecord);
    free(cmpRecord);
    close(fileDesc);
}

void copy_lib(char* from, char* to, int number, int size){
    FILE* src = fopen(from, "r");
    FILE* dest = fopen(to, "w");
    
    char* buf = malloc(size * sizeof(char));
    for(int i=0;i<number;i++){
        if(fread(buf, sizeof(char), size, src) != size){
            printf("Reading failed!\n");
            return;
        }
        if(fwrite(buf, sizeof(char), size, dest) != size){
            printf("Writing failed!\n");
            return;
        }
    }
    free(buf);
    fclose(dest);
    fclose(src);
}

void sort_lib(char* fileName, int number, int size){
    FILE* file = fopen(fileName, "r+");
    char* sortRecord = malloc(size * sizeof(char));                     // element being sorted
    char* cmpRecord = malloc(size * sizeof(char));                      // element to compare to 
    int sortKey, cmpKey;
    long int offset = (long int) size * sizeof(char);

    for(int i=1;i<number;i++){
        fseek(file, offset * i, 0);                                     // picking element (skip first) 
        if(fread(sortRecord, sizeof(char), size, file) != size){
            printf("Error while reading file!\n");
            return;
        }

        fseek(file, offset * (-2), 1);                                  // element before one being sorted atm
        if(fread(cmpRecord, sizeof(char), size, file) != size){
            printf("Error while reading file!\n");
            return;
        }
        
        sortKey = (int) sortRecord[0];
        cmpKey = (int) cmpRecord[0];

        int c = i;                                                      // counter how many element before the sorted one

        while(sortKey < cmpKey && c != 1){
            
            if(fwrite(cmpRecord, sizeof(char), size, file) != size){    // moving elements to the right
                printf("Error while writing to file!\n");
                return;
            }

            fseek(file, offset * (-3), 1);                              // back one element
            if(fread(cmpRecord, sizeof(char), size, file) != size){
                printf("Error while reading file!\n");
                return;
            }
            cmpKey = (int) cmpRecord[0];
            c--;                                                        // counting until we cant move back more
        }

        if(sortKey < cmpKey){                                           // if the first element is greater than sorted one
            if(fwrite(cmpRecord, sizeof(char), size, file) != size){    // write it on the second place
                printf("Error while writing to file!\n");
                return;
            }
            fseek(file, offset * (-2), SEEK_CUR);                       // move the pointer to write sorted element on the first place
        }
        
        if(fwrite(sortRecord, sizeof(char), size, file) != size){       // writing sorted elememt
            printf("Error while writing to file!\n");
            return;
        }
        
    }

    free(sortRecord);
    free(cmpRecord);
    fclose(file);
}



int main(int argc, char **argv){
    // argv[0] - program call
    srand(time(NULL));
    char* pEnd;
    int number, size;
    if(strcmp(argv[1],"generate") == 0){
        char* fileName = argv[2];
        number = (int) strtol(argv[3], &pEnd, 10);
        size = (int) strtol(argv[4], &pEnd, 10);
        if(number <= 0 || size <= 0){
            printf("Wrong number or size!\n");
            return 1;
        }
        generate(fileName, number, size);
    }

    else if(strcmp(argv[1],"sort") == 0){
        char* fileName = argv[2];
        number = (int) strtol(argv[3], &pEnd, 10);
        size = (int) strtol(argv[4], &pEnd, 10);
        if(number <= 0 || size <= 0){
            printf("Wrong number or size!\n");
            return 1;
        }
        if(strcmp(argv[5],"sys") == 0){
            struct fullTime makeStamp1 = mkTimeStamp();
            sort_sys(fileName, number, size);               // SORT_SYS
            struct fullTime makeStamp2 = mkTimeStamp();
            printf("Sort_sys %d records of %d size: \nUser time: %f \tSystem time: %f\n\n",
                number, size,
                (float)(makeStamp2.timeU-makeStamp1.timeU)/(float)sysconf(_SC_CLK_TCK),
                (float)(makeStamp2.timeS-makeStamp1.timeS)/(float)sysconf(_SC_CLK_TCK));
        }
        else if(strcmp(argv[5],"lib") == 0){
            struct fullTime makeStamp1 = mkTimeStamp();
            sort_lib(fileName, number, size);                       // SORT_LIB
            struct fullTime makeStamp2 = mkTimeStamp();
            printf("Sort_lib %d records of %d size: \nUser time: %f \tSystem time: %f\n\n",
                number, size,
                (float)(makeStamp2.timeU-makeStamp1.timeU)/(float)sysconf(_SC_CLK_TCK),
                (float)(makeStamp2.timeS-makeStamp1.timeS)/(float)sysconf(_SC_CLK_TCK));
        }
        else{
            printf("As fourth give either 'sys' or 'lib'!\n");
            return 1;            
        }
    }

    else if(strcmp(argv[1],"copy") == 0){
        char* from = argv[2];
        char* to = argv[3];
        number = (int) strtol(argv[4], &pEnd, 10);
        size = (int) strtol(argv[5], &pEnd, 10);
        if(number <= 0 || size <= 0){
            printf("Wrong number or size!\n");
            return 1;
        }
        if(strcmp(argv[6],"sys") == 0){
            struct fullTime makeStamp1 = mkTimeStamp();
            copy_sys(from, to, number, size);                       // COPY_SYS
            struct fullTime makeStamp2 = mkTimeStamp();
            printf("Copy_sys %d records of %d size: \nUser time: %f \tSystem time: %f\n\n",
                number, size,
                (float)(makeStamp2.timeU-makeStamp1.timeU)/(float)sysconf(_SC_CLK_TCK),
                (float)(makeStamp2.timeS-makeStamp1.timeS)/(float)sysconf(_SC_CLK_TCK));
        }
        else if(strcmp(argv[6],"lib") == 0){
            struct fullTime makeStamp1 = mkTimeStamp();
            copy_lib(from, to, number, size);                       // COPY_LIB
            struct fullTime makeStamp2 = mkTimeStamp();
            printf("Copy_lib %d records of %d size: \nUser time: %f \tSystem time: %f\n\n",
                number, size,
                (float)(makeStamp2.timeU-makeStamp1.timeU)/(float)sysconf(_SC_CLK_TCK),
                (float)(makeStamp2.timeS-makeStamp1.timeS)/(float)sysconf(_SC_CLK_TCK));
        }
        else{
            printf("As fifth argument give either 'sys' or 'lib'!\n");
            return 1;            
        }

    }
    else{
        printf("Pass in the first argument: 'generate', 'copy' or 'sort'\n");
        return 1;
    }
    return 0;
}
