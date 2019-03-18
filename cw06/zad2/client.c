#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <mqueue.h>

#include "msgSpecs.h"

int serverQueue = -1;           // main queue ID
int queue = -1;                 // client queue ID
char name[10];                  // client queue name


void closeQueue(){
    if(serverQueue > -1){       // server queue must exist
        
        msgbuf request;
        request.mtype = STOP;
        request.pid = getpid();
        
        if(mq_send(serverQueue,(char*)&request,MSGSIZE,1) != 0) printf("Didn't send STOP request. SIGINT was from server and main queue is closed.\n");
        else printf("Sent STOP request to server.\n");
        if(mq_close(serverQueue) != 0) printf("Couldn't close the server queue!.\n");
    }
    if(queue > -1){             // dont do it if queue didnt exist 
        if(mq_close(queue) != 0) printf("Couldn't close the client queue!.\n");
        else if(mq_unlink(name) != 0) printf("Couldn't remove the client queue!.\n");
        else printf("Client queue closed. Exiting...\n");
    }
}

void handleSIGINT(int signo){
    printf("Received SIGINT.\n");
    exit(EXIT_SUCCESS);
}

void requestMirror(){

    msgbuf request;
    request.mtype = MIRROR;
    request.pid = getpid();

    printf("Type the line to be mirrored:\n");
    fgets(request.message, TEXTSIZE, stdin);

    if(mq_send(serverQueue,(char*)&request,MSGSIZE,1) != 0){
        printf("Error while sending MIRROR request!\n");
        exit(EXIT_FAILURE);
    }

    if(mq_receive(queue,(char*)&request,MSGSIZE,NULL) == -1){
        printf("Error while receiving MIRROR from queue!\n");
        exit(EXIT_FAILURE);
    }

    printf("%s\n",request.message);
}

void requestCalc(){
    
    msgbuf request;
    request.mtype = CALC;
    request.pid = getpid();
    printf("Type the operation(ADD,SUB,MUL,DIV) and two arguments:\n");
    fgets(request.message, TEXTSIZE, stdin);
    
    if(mq_send(serverQueue,(char*)&request,MSGSIZE,1) != 0){
        printf("Error while sending CALC request!\n");
        exit(EXIT_FAILURE);
    }

    if(mq_receive(queue,(char*)&request,MSGSIZE,NULL) == -1){
        printf("Error while receiving CALC from queue!\n");
        exit(EXIT_FAILURE);
    }

    printf("%s\n",request.message);
}

void requestTime(){
    
    msgbuf request;
    request.mtype = TIME;
    request.pid = getpid();

    if(mq_send(serverQueue,(char*)&request,MSGSIZE,1) != 0){
        printf("Error while sending TIME request!\n");
        exit(EXIT_FAILURE);
    }

    if(mq_receive(queue,(char*)&request,MSGSIZE,NULL) == -1){
        printf("Error while receiving TIME from queue!\n");
        exit(EXIT_FAILURE);
    }

    printf("%s\n",request.message);
}

void requestEnd(){
    
    msgbuf request;
    request.mtype = END;
    request.pid = getpid();

    if(mq_send(serverQueue,(char*)&request,MSGSIZE,1) != 0){
        printf("Error while sending END request!\n");
        exit(EXIT_FAILURE);
    }
}

int main(){
    
    atexit(closeQueue);

    signal(SIGINT,handleSIGINT);

    struct mq_attr attributes;
    attributes.mq_maxmsg = MSGNUMBER;
    attributes.mq_msgsize = MSGSIZE;

    serverQueue = mq_open(QNAME, O_WRONLY);     
                    // server queue
    if(serverQueue < 0){
        printf("Couldn't open server queue.\n");
        return EXIT_FAILURE;
    }
    
    sprintf(name,"/%d",getpid());
    
    queue = mq_open(name, O_RDONLY | O_CREAT | O_EXCL, 0666, &attributes);
                    // client queue
    if(queue < 0){
        printf("Couldn't set up client queue.\n");
        return EXIT_FAILURE;
    }

    msgbuf msg;
    msg.mtype = START;
    msg.pid = getpid();
    sprintf(msg.message,"%s",name);

    if(mq_send(serverQueue,(char*)&msg,MSGSIZE,1) != 0){
        printf("Error while sending START request!\n");
        return EXIT_FAILURE;
    }

    if(mq_receive(queue,(char*)&msg,MSGSIZE,NULL) == -1){
        printf("Error while receiving ID from queue!\n");
        return EXIT_FAILURE;
    }

    int ID = (int) strtol(msg.message,NULL,10);

    if(ID < 0){
        printf("Server didnt set up connection!\n");
        return EXIT_FAILURE;
    }
    else printf("Received ID:%d from server.\n",ID);

    char task[10];
    printf("1 - MIRROR, 2 - CALC, 3 - TIME, 4 - END\n");

    while(1){
        
        printf("Choose the task:\n");
        fgets(task, 10, stdin);
        int length = strlen(task);
        if (task[length-1] == '\n') task[length-1] = 0;
        
        if(strcmp(task,"MIRROR") == 0 || strcmp(task,"1") == 0) 
            requestMirror();
        else if(strcmp(task,"CALC") == 0 || strcmp(task,"2") == 0) 
            requestCalc();
        else if(strcmp(task,"TIME") == 0 || strcmp(task,"3") == 0) 
            requestTime();
        else if(strcmp(task,"END") == 0 || strcmp(task,"4") == 0) 
            requestEnd();
        else printf("Wrong request! (1 - MIRROR, 2 - CALC, 3 - TIME, 4 - END)\n");

    }

    return EXIT_SUCCESS;
}