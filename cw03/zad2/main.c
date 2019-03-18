#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define maxLine 100
#define maxArgs 20

int main(int argc, char** argv){
    
    if(argc != 2){
        printf("Give a filename (ONE!) as an argument!\n");
        return 1;
    }
    
    FILE* file = fopen(argv[1], "r");
    if(file == NULL){
        printf("Error while opening file!\n");
        return 1;
    }

    char fileLine[maxLine];             // buffor for single line from file
    char* arguments[maxArgs];
    int argCounter;
    int i=0;
    
    while(fgets(fileLine, maxLine, file) != NULL){
        argCounter = 0;
        
        while((arguments[argCounter] = strtok(argCounter == 0 ? fileLine : NULL, " \n\t")) != NULL){
            argCounter++;                                                       // reading arguments
            if(argCounter == maxArgs){
                printf("Too many arguments! Reading only %d.", maxArgs);
                break;
            }
        }
        i++;
        printf("%d: %s\n",i,fileLine);
        int pid = (int) fork();                 // new process
        if(pid == 0) {
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

    fclose(file);
    return 0;
}