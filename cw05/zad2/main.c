#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

#define maxSlaves 100

int main(int argc, char **argv){

    if(argc != 4){
        printf("Give pipename, number of slaves and N parameter!\n");
        return 1;
    }

    // pipename - argv[1]
    int slaves = (int) strtol(argv[2], NULL, 10);
    int N = (int) strtol(argv[3], NULL, 10);

    if(slaves > maxSlaves || N < 1){
        printf("Wrong input!\n");
        return 1;
    }

    int children[slaves];
    
    int masterPid = (int) fork();
    if (masterPid == 0){
        execlp("./master", "master", argv[1], NULL);
    }
    
    sleep(1);   // for setup

    for (int i = 0; i < slaves; ++i) {
        children[i] = (int) fork();
        if (children[i] == 0){
            execlp("./slave","slave", argv[1], argv[3], NULL);
        }
    }
    

    for (int i = 0; i < slaves; ++i) {     // wait for all children to finish
        waitpid(children[i], NULL, 0);
    }

    kill(masterPid, SIGINT);    // exit master
    waitpid(masterPid, NULL, 0);

    return 0;


}
