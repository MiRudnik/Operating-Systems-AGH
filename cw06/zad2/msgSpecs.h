#ifndef msgSpecs_h
#define msgSpecs_h

#define MAXCLIENTS 10
#define TEXTSIZE 256
#define MSGNUMBER 10

typedef enum mtype{
    MIRROR = 1,
    CALC = 2,
    TIME = 3,
    END = 4,
    START = 5,
    STOP = 6
} mtype;

typedef struct msgbuf{
    long mtype;
    int pid;
    char message[TEXTSIZE];
} msgbuf;

const int MSGSIZE = sizeof(msgbuf);
const char QNAME[] = "/mainQueue";

#endif