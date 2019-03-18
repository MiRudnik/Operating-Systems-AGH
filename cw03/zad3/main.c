#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>

#define maxLine 100
#define maxArgs 20

void setLimits(long int cpuLimit, long int memLimit){

   struct rlimit cpuRLimit;
   struct rlimit memRLimit;

    cpuRLimit.rlim_max = (rlim_t) cpuLimit;
    cpuRLimit.rlim_cur = (rlim_t) cpuLimit;
    memRLimit.rlim_max = (rlim_t) memLimit;
    memRLimit.rlim_cur = (rlim_t) memLimit;

    if(setrlimit(RLIMIT_CPU, &cpuRLimit) == -1){
        printf("Error while setting cpu limit!\n");
    }
    if(setrlimit(RLIMIT_DATA, &memRLimit) == -1){
        printf("Error while setting memory limit!\n");
    }
    if(setrlimit(RLIMIT_STACK, &memRLimit) == -1){
        printf("Error while setting memory limit!\n");
    }
}

int main(int argc, char** argv){
    
    if(argc != 4){
        printf("Give a filename, timelimit(secs) and memorylimit(mbs) as arguments!\n");
        return 1;
    }
    
    FILE* file = fopen(argv[1], "r");
    if(file == NULL){
        printf("Error while opening file!\n");
        return 1;
    }

    struct rusage begUsage;
    getrusage(RUSAGE_CHILDREN, &begUsage);                                      // resource usage
    
    char fileLine[maxLine];                                                     // buffor for single line from file
    char* arguments[maxArgs];
    int argCounter;

    long int cpuLimit = strtol(argv[2], NULL, 10);
    long int memLimit = strtol(argv[3], NULL, 10)*1024*1024;                    // bytes to megabytes
    
    while(fgets(fileLine, maxLine, file) != NULL){
        argCounter = 0;
        
        while((arguments[argCounter] = strtok(argCounter == 0 ? fileLine : NULL, " \n\t")) != NULL){
            argCounter++;                                                       // reading arguments
            if(argCounter == maxArgs){
                printf("Too many arguments! Reading only %d.", maxArgs);
                break;
            }
        }
        
        
        pid_t pid = fork();                 // new process
        if(pid == 0) {
            setLimits(cpuLimit, memLimit);
            execvp(arguments[0], arguments);
            exit(1);
        }
        int status;
        wait(&status);
        if(status != 0){
            printf("Error while executing:  ");
            for(int i = 0; i < argCounter; i++) {
                printf("%s ", arguments[i]);
            }
            printf("\n");
            return 1;
        }
    }
    struct rusage endUsage;
    getrusage(RUSAGE_CHILDREN, &endUsage);
    struct timeval userTime;                // tv_sec - seconds
    struct timeval sysTime;                 // tv_usec - microseconds
    timersub(&endUsage.ru_utime, &begUsage.ru_utime, &userTime);    // end - beg = userTime
    timersub(&endUsage.ru_stime, &begUsage.ru_stime, &sysTime);     // end - beg = sysTime
    begUsage = endUsage;
    for (int i = 0; i < argCounter; i++) {
        printf("%s ", arguments[i]);
    }
    printf("\n_______________________________________________\n");
    printf("User CPU time used: %d.%d seconds \nSystem CPU time used: %d.%d seconds\n\n", 
            (int) userTime.tv_sec, (int) userTime.tv_usec, (int) sysTime.tv_sec, (int) sysTime.tv_usec);

    fclose(file);
    return 0;
}