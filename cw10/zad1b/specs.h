#ifndef SPECS_H
#define MAX_LEN 100
#define MAX_CLIENTS 15
#define MAX_EVENTS 5
#define INET 0
#define LOCAL 1
#define START 2
#define ORDER 3
#define PING 4
#define ADD 5
#define SUB 6
#define MUL 7
#define DIV 8
#define END 9

struct client{
    char name[MAX_LEN];
    struct sockaddr addr;
    socklen_t addr_size;
    int fd;
    int pings;
};

struct order{
    int operation;
    int operand1;
    int operand2;
};

struct msg{
    int msg_type;
    int id;
    char name[MAX_LEN];
    struct order msg_order;
};
#define SPECS_H

#endif