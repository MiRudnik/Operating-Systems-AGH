#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "msgSpecs.h"

int clients[MAXCLIENTS][2];         // 0 - queueID, 1 - pid
int currentClients = 0;             // counter
int running = 1;                    // to end the queue
int serverQueue = -1;               // main queue


void closeQueues(){
    if(serverQueue > -1){           // dont do it if queue didnt exist
        for(int i=0;i<currentClients;i++){
            kill(clients[i][1], SIGINT);
        }
        printf("Closing server.\n");
        if(msgctl(serverQueue,IPC_RMID,NULL) != 0) printf("Couldnt remove the main queue!.\n");
    }
}

void handleSIGINT(int signo){
    printf("Received SIGINT.\n");
    exit(EXIT_SUCCESS);
}

int getClientQueue(int pid){
    
    for(int i=0;i<currentClients;i++){
        if(clients[i][1] == pid) return clients[i][0];
    }
    return -1;
}

void handleMirror(msgbuf *msg){
    printf("Received MIRROR task from process %d.\n",msg->pid);

    int queue = getClientQueue(msg->pid);
    if(queue == -1){
        printf("Couldn't find clients queue.\n");
        exit(EXIT_FAILURE);
    }

    char *base = strtok(msg->message,"\t\n");
    int size = strlen(base);
    char mirrored[size];
    
    for(int i=size-1;i>=0;i--){
        mirrored[size-1-i] = base[i];
    }

    msgbuf response;
    response.mtype = msg->pid;
    response.pid = getpid();
    sprintf(response.message,"%s",mirrored);

    if(msgsnd(queue,&response,MSGSIZE,0) != 0){
        printf("Error while sending mirrored message!\n");
        exit(EXIT_FAILURE);
    }

}

void handleCalc(msgbuf *msg){
    printf("Received CALC task from process %d.\n",msg->pid);

    int queue = getClientQueue(msg->pid);
    if(queue == -1){
        printf("Couldn't find clients queue.\n");
        exit(EXIT_FAILURE);
    }
    
    msgbuf response;
    response.mtype = msg->pid;
    response.pid = getpid();

    char *operation = strtok(msg->message," \t\n");
    char *firstArg = strtok(NULL," \t\n");

    char *secondArg = strtok(NULL," \t\n");
    if(firstArg == NULL || secondArg == NULL){              // not enough arguments
        sprintf(response.message,"Not enough arguments!\n");
        if(msgsnd(queue,&response,MSGSIZE,0) != 0){
            printf("Error while sending answer!\n");
            exit(EXIT_FAILURE);
        }
        return;
    }

    if(strtok(NULL," \t\n") != NULL){                       // more than 2 arguments
        sprintf(response.message,"Too many arguments!\n");
        if(msgsnd(queue,&response,MSGSIZE,0) != 0){
            printf("Error while sending answer!\n");
            exit(EXIT_FAILURE);
        }
        return;
    }

    char *end1;
    char *end2;  // to validate numbers

    int firstNumber = (int) strtol(firstArg,&end1,10);
    int secondNumber = (int) strtol(secondArg,&end2,10);

    if ((end1 == firstArg) || (*end1 != '\0')) sprintf(response.message,"First argument is not valid!\n");          // first argument wasn't a number
    else if((end2 == secondArg) || (*end2 != '\0')) sprintf(response.message,"Second argument is not valid!\n");    // second argument wasn't a number
    else if(strcmp(operation,"ADD") == 0) sprintf(response.message,"%d + %d = %d\n",firstNumber,secondNumber,firstNumber+secondNumber);
    else if(strcmp(operation,"SUB") == 0) sprintf(response.message,"%d - %d = %d\n",firstNumber,secondNumber,firstNumber-secondNumber);
    else if(strcmp(operation,"MUL") == 0) sprintf(response.message,"%d * %d = %d\n",firstNumber,secondNumber,firstNumber*secondNumber);
    else if(strcmp(operation,"DIV") == 0 && secondNumber != 0) sprintf(response.message,"%d / %d = %f\n",
                                                                                firstNumber,secondNumber,(float)firstNumber/(float)secondNumber);
    else sprintf(response.message,"Invalid operation\n");
   
    if(msgsnd(queue,&response,MSGSIZE,0) != 0){
        printf("Error while sending answer!\n");
        exit(EXIT_FAILURE);
    }
    
}

void handleTime(msgbuf *msg){
    printf("Received TIME task from process %d.\n",msg->pid);

    int queue = getClientQueue(msg->pid);
    if(queue == -1){
        printf("Couldn't find clients queue.\n");
        exit(EXIT_FAILURE);
    }

    char parsedTime[20];
    time_t now = time(NULL);
    struct tm *stamp = localtime(&now);
    strftime (parsedTime, 100, "%c", stamp);

    msgbuf response;
    response.mtype = msg->pid;
    response.pid = getpid();
    sprintf(response.message,"%s",parsedTime);

    if(msgsnd(queue,&response,MSGSIZE,0) != 0){
        printf("Error while sending time!\n");
        exit(EXIT_FAILURE);
    }
}

void handleEnd(msgbuf *msg){
    printf("Received END task from process %d.\n",msg->pid);
    running = 0;
    printf("Program will end once queue is empty.\n");
}

void handleStart(msgbuf *msg){
    
    key_t clientKey = (key_t) strtol(msg->message,NULL,10);
    int clientQueue = msgget(clientKey, 0);
    msgbuf response;
    response.mtype = msg->pid;
    response.pid = getpid();
    
    if(currentClients >= MAXCLIENTS){
        printf("Too many clients!\n");
        sprintf(response.message,"%d",-1);        
    }
    else{
        sprintf(response.message,"%d",currentClients);
        clients[currentClients][0] = clientQueue;
        clients[currentClients][1] = msg->pid;
        
        printf("Added process %d as client with ID: %d\n",msg->pid,currentClients);
        currentClients++;
    }
    if(msgsnd(clientQueue,&response,MSGSIZE,0) != 0){
        printf("Error while sending ID!\n");
        exit(EXIT_FAILURE);
    }  
}

void handleStop(msgbuf *msg){
    
    int clientID;
    for(int i=0;i<currentClients;i++){
        if(clients[i][1] == msg->pid){
            clientID = i;
            break;
        }
    }
    for(int i=clientID;i<currentClients-1;i++){
        clients[i][0] = clients[i+1][0];
        clients[i][1] = clients[i+1][1];
    }
    clients[currentClients][0] = 0;
    clients[currentClients][1] = 0;
    currentClients--;
    printf("Removed client with pid: %d.\n",msg->pid);
}

// ________________________________MAIN________________________________

int main(){
    
    atexit(closeQueues);

    signal(SIGINT,handleSIGINT);

    char* path = getenv("HOME");
    if(path == NULL){
        printf("Couldnt get $HOME value.\n");
        return EXIT_FAILURE;
    }
    
    key_t serverKey = ftok(path,PROJECTID);

    serverQueue = msgget(serverKey, IPC_CREAT | IPC_EXCL | 0666);
    if(serverQueue < 0){
        printf("Couldn't set up first queue.\n");
        return EXIT_FAILURE;
    }

    struct msqid_ds stats;
    msgbuf msg;

    while(1){
        
        if(running == 0){
            if(msgctl(serverQueue,IPC_STAT,&stats) != 0){
                printf("Error while checking queue status!\n");
                return EXIT_FAILURE;
            }
            if(stats.msg_qnum == 0) break;
        }

        if(msgrcv(serverQueue,&msg,MSGSIZE,0,0) == -1){
            printf("Error while receiving from queue!\n");
            return EXIT_FAILURE;
        }

        switch(msg.mtype){
            case MIRROR:
                handleMirror(&msg);
                break;
            case CALC:
                handleCalc(&msg);
                break;
            case TIME:
                handleTime(&msg);
                break;
            case END:
                handleEnd(&msg);
                break;
            case START:
                handleStart(&msg);
                break;
            case STOP:
                handleStop(&msg);
                break;
            default:
                printf("Wrong request!\n");
        }

    }

    return EXIT_SUCCESS;
}