#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <sys/un.h>

#include "specs.h"

void main_process_function ();

char *name;
char *address;
unsigned short port;
int type;
int socketd;


int calculate_expr(int operation, int operand1, int operand2){
    if (operation == ADD){
        return operand1+operand2;
    }
    else if (operation == SUB){
        return operand1-operand2;
    }
    else if (operation == MUL){
        return operand1*operand2;
    }
    else if (operation == DIV){
        if (operand2 == 0){
            printf("second argument should't be 0 in DIV operation. Giving -1 as result.\n");
            fflush(stdout);
            return -1;
        }
        return operand1/operand2;
    }
    else{
        printf("Wrong expression passed. Giving -1 as a result.\n");
        fflush(stdout);
        return -1;
    }
}

void cleanup(){
    shutdown(socketd, SHUT_RDWR);
    close(socketd);
}

void inthandler(int signo){
    if (signo == SIGINT)
    {
        printf("Received SIGINT. Exiting.\n");
        exit(EXIT_SUCCESS);
    }
}

void respond_to_order(struct order order, int id){

    struct msg msg;
    strcpy(msg.name, name);
    msg.msg_type = ORDER;
    msg.id = id;
    int result = calculate_expr(order.operation, order.operand1, order.operand2);
    msg.msg_order.operand1 = result;
    printf("Calculated expression and sending it's result(%d) to server.\n", msg.msg_order.operand1);
    fflush(stdout);
    if(write(socketd, &msg, sizeof(msg)) < 0){
        printf("Couldn't send calc result to server in client %s.\n", name);
        exit(EXIT_FAILURE);
    }
}

void main_process_function(){

    while(1) {
        struct msg msg;
        ssize_t quantity = recv(socketd, &msg, sizeof(msg), MSG_WAITALL);
        if (quantity == 0){
            printf("Server stopped.\n");
            exit(EXIT_SUCCESS);
        }
        switch (msg.msg_type){
            case START:
                printf("Shouldn't have received this. Client with that name already exists.\n");
                exit(EXIT_FAILURE);
                break;
            case ORDER:
                fflush(stdout);
                respond_to_order(msg.msg_order, msg.id);
                break;
            case PING:
                printf("Got pinged by server.\n");
                fflush(stdout);
                if(write(socketd, &msg, sizeof(msg)) < 0){
                    printf("Couldn't send ping to server in client %s.\n", name);
                    exit(EXIT_FAILURE);
                }
                break;
            case END:
                printf("Server is down.\n");
                exit(EXIT_SUCCESS);
            default:
                printf("I got something strange from server: %d. Exiting.\n", msg.msg_type);
                exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[]){

    if (argc < 4){
        printf("Pass name, type, address [,port]!\n");
        exit(EXIT_FAILURE);
    }

    name = argv[1];
    type = (int) strtol(argv[2], NULL, 10);
    address = argv[3];

    if (type == INET){ // type 0 in console
    
        if(argc != 5){
            printf("Pass also port for INET.\n");
            exit(EXIT_FAILURE);
        }

        port = (unsigned short) strtoul(argv[4], NULL, 10);
        socketd = socket(AF_INET, SOCK_DGRAM, 0);
        if (socketd < 0){
            printf("Couldn't create INET socket.\n");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        if (inet_pton(AF_INET, address, &addr.sin_addr) == 0){
            printf("Couldn't pton in INET.\n");
            exit(EXIT_FAILURE);
        }

        addr.sin_port = htons(port);
        if (connect(socketd, (const struct sockaddr *)&addr, sizeof(addr)) < 0){
            printf("Couldn't connect INET.\n");
            exit(EXIT_FAILURE);
        }
    }
    else if (type == LOCAL){ // type 1 in console
    
        socketd = socket(AF_UNIX, SOCK_DGRAM, 0);
        if (socketd < 0){
            printf("Couldn't create LOCAL socket.\n");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_un addr;
        addr.sun_family = AF_UNIX;
        strcpy(addr.sun_path, address);
        if(bind(socketd, (const struct sockaddr *)&addr, sizeof(sa_family_t)) < 0){
            printf("Couldn't bind LOCAL.\n");   // needed in UNIX
            exit(EXIT_FAILURE);
        }

        if(connect(socketd, (const struct sockaddr *)&addr, sizeof(addr)) < 0){
            printf("Couldn't connect LOCAL.\n");
            exit(EXIT_FAILURE);
        }
    }

    atexit(cleanup);
    signal(SIGINT, inthandler);

    struct msg msg;
    strcpy(msg.name, name);
    msg.msg_type = START;
    if(write(socketd, &msg, sizeof(msg)) < 0){
        printf("Couldn't start connection in client with name %s", name);
        exit(EXIT_FAILURE);
    }
    
    printf("Should now be able to receive order in %s\n", name);
    fflush(stdout);

    main_process_function();

    return 0;
}