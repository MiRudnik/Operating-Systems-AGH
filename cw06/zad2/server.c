#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <mqueue.h>

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
        if(mq_close(serverQueue) != 0) printf("Couldn't close the server queue!.\n");
        else if(mq_unlink(QNAME) != 0) printf("Couldn't remove the server queue!.\n");
        else printf("Server queue closed. Exiting...\n");
    }
}

void handleSIGINT(int signo){
    printf("Received SIGINT.\n");
    exit(EXIT_SUCCESS);         // OK
}

int getClientQueue(int pid){
    
    for(int i=0;i<currentClients;i++){
        if(clients[i][1] == pid) return clients[i][0];
    }
    return -1;              // OK
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

    if(mq_send(queue,(char*)&response,MSGSIZE,1) != 0){
        printf("Error while sending mirrored message!\n");
        exit(EXIT_FAILURE);
    }           // OK

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
        if(mq_send(queue,(char*)&response,MSGSIZE,1) != 0){
            printf("Error while sending answer!\n");
            exit(EXIT_FAILURE);
        }
        return;
    }

    if(strtok(NULL," \t\n") != NULL){                       // more than 2 arguments
        sprintf(response.message,"Too many arguments!\n");
        if(mq_send(queue,(char*)&response,MSGSIZE,1) != 0){
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
   
    if(mq_send(queue,(char*)&response,MSGSIZE,1) != 0){
        printf("Error while sending answer!\n");
        exit(EXIT_FAILURE);             // OK
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

    if(mq_send(queue,(char*)&response,MSGSIZE,1) != 0){
        printf("Error while sending time!\n");
        exit(EXIT_FAILURE);
    }           // OK
}

void handleEnd(msgbuf *msg){
    printf("Received END task from process %d.\n",msg->pid);
    running = 0;
    printf("Program will end once queue is empty.\n");      // OK
}

void handleStart(msgbuf *msg){
    
    int clientQueue = mq_open(msg->message, O_WRONLY);

    if(clientQueue < 0){
        printf("Couldn't set up clients queue.\n");
        exit(EXIT_FAILURE);
    }

    msgbuf response;
    response.mtype = msg->pid;
    response.pid = getpid();
    
    if(currentClients >= MAXCLIENTS){
        printf("Too many clients!\n");
        sprintf(response.message,"%d",-1);

        if(mq_send(clientQueue,(char*)&response,MSGSIZE,1) != 0){
            printf("Error while sending ID!\n");
            exit(EXIT_FAILURE);
        }

        if(mq_close(clientQueue) != 0){
            printf("Error while closing client queue!\n");
            exit(EXIT_FAILURE);
        }       
    }
    else{
        sprintf(response.message,"%d",currentClients);
        clients[currentClients][0] = clientQueue;
        clients[currentClients][1] = msg->pid;
        
        printf("Added process %d as client with ID: %d\n",msg->pid,currentClients);
        currentClients++;
        
        if(mq_send(clientQueue,(char*)&response,MSGSIZE,1) != 0){
            printf("Error while sending ID!\n");
            exit(EXIT_FAILURE);         // OK
        }
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

    if(mq_close(clients[clientID][0]) != 0){
        printf("Error while closing client queue!\n");
        exit(EXIT_FAILURE);
    } 

    for(int i=clientID;i<currentClients-1;i++){
        clients[i][0] = clients[i+1][0];
        clients[i][1] = clients[i+1][1];
    }
    clients[currentClients][0] = 0;
    clients[currentClients][1] = 0;
    currentClients--;
    printf("Removed client with pid: %d.\n",msg->pid);  // OK
}

// _________________________________MAIN_____________________________________

int main(){

    atexit(closeQueues);

    signal(SIGINT,handleSIGINT);

    struct mq_attr attributes;
    attributes.mq_maxmsg = MSGNUMBER;
    attributes.mq_msgsize = MSGSIZE;

    serverQueue = mq_open(QNAME, O_RDONLY | O_CREAT | O_EXCL, 0666, &attributes);

    if(serverQueue < 0){
        printf("Couldn't set up first queue.\n");
        return EXIT_FAILURE;
    }

    struct mq_attr stats;
    msgbuf msg;

    while(1){
        
        if(running == 0){
            if(mq_getattr(serverQueue,&stats) != 0){
                printf("Error while checking queue status!\n");
                return EXIT_FAILURE;
            }
            if(stats.mq_curmsgs == 0) break;
        }

        if(mq_receive(serverQueue,(char*)&msg,MSGSIZE,NULL) == -1){ // 2 attribute char*
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