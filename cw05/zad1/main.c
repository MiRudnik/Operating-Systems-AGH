#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define maxLine 100
#define maxArgs 20
#define maxProgs 10

char **arguments;

char** parseArgs(char* programLine){
    int argCounter = 0;
    arguments = malloc(sizeof(char*) * maxArgs);

    while((arguments[argCounter] = strtok(argCounter == 0 ? programLine : NULL, " \n\t")) != NULL){
        argCounter++;                                                       // reading arguments
        if(argCounter == maxArgs){
            printf("Too many arguments! Reading only %d.", maxArgs);
            break;
        }
    }

    return arguments;
}

void execLine(char* line){
    int progCounter = 0;
    char *programs[maxProgs];
    char **parsedProg;
    int firstPipe[2];
    int secondPipe[2];
    int *writePipe, *readPipe;

    while((programs[progCounter] = strtok(progCounter == 0 ? line : NULL, "|")) != NULL){
        progCounter++;                                                       // reading programs
        if(progCounter == maxProgs){
            printf("Too many programs! Reading only %d.", maxProgs);
            break;
        }
    }

    for(int i=0; i<progCounter; i++){       // executing programs

        parsedProg =  parseArgs(programs[i]);

        if(i % 2 == 0){                     // switching between pipes
            writePipe = firstPipe;
            readPipe = secondPipe;
        }
        else{
            writePipe = secondPipe;
            readPipe = firstPipe;
        }    
        
        if(i != 0){                         // to open new pipe
            close(writePipe[0]);
            close(writePipe[1]);
        }

        pipe(writePipe);

        pid_t child = fork();
        if(child == 0) {
            if(i != progCounter-1){                     // every but last
                close(writePipe[0]);                    // dont read from pipe1
                dup2(writePipe[1], STDOUT_FILENO);      // write to pipe1 instead of stdout
            }

            if(i != 0){                                 // every but first
                close(readPipe[1]);                     // dont write to pipe2
                dup2(readPipe[0], STDIN_FILENO);        // read from pipe2 instead of stdin
            }

            execvp(parsedProg[0], parsedProg);
            free(arguments);
            exit(EXIT_SUCCESS);
        }
        
    }
    wait(NULL);
    close(writePipe[0]);
    close(writePipe[1]);                // closing last pipe
}


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

    int i=0;
    char fileLine[maxLine];                 // buffor for single line from file
        
    while(fgets(fileLine, maxLine, file) != NULL){
        
        i++;        
        printf("%d: %s\n",i,fileLine);
        pid_t pid = fork();                 // new process
        if(pid == 0) {
            execLine(fileLine);
            _exit(0);
        }
        wait(NULL);
        
    }

    fclose(file);
    return 0;
}