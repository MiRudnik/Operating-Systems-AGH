#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#define maxLine 110

FILE *pipeHandler;

void finish(int signo){
    printf("\nFinishing reading...\n");
    fclose(pipeHandler);
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv){

    mkfifo(argv[1], 0777);  // 0777 all rights for everyone
    char buffer[maxLine];

    struct sigaction actions;
    sigemptyset(&actions.sa_mask);
    actions.sa_flags = 0;
    actions.sa_handler = finish;
    sigaction(SIGINT, &actions, NULL);

    pipeHandler = fopen(argv[1], "r");
    while (fgets(buffer, maxLine, pipeHandler) != NULL) {
        printf("%s\n", buffer);
    }
    fclose(pipeHandler);
    return 0;
}