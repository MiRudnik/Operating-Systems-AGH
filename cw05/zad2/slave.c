#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define maxLine 100

int main(int argc, char **argv){

    srand(time(NULL));
    
    FILE *pipe = fopen(argv[1], "w");
    int N = (int) strtol(argv[2], NULL, 10);
    char line[maxLine];
    char formatted[maxLine+10];

    int pid = (int) getpid();
    int sleepTime;

    for (int i = 0; i < N; i++) {
        FILE *date = popen("date", "r");        // reading output from command date
        fgets(line, maxLine, date);
        sprintf(formatted, "Slave pid: %d - %s", pid, line);
        fwrite(formatted, sizeof(char), strlen(formatted), pipe);
        sleepTime = rand()%4 + 1;           // 1 - 4 secs
        sleep(sleepTime);
    }
    fclose(pipe);
    return 0;

}