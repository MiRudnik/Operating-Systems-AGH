#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <sys/times.h>
#include <math.h>   // -lm to linker

#define MAXBUF 512

typedef struct thread_args{
    int W;
    int H;
    int C;
    int **pgm;
    float **filter;
    int **result;
    int from;       // specifying which rows to calculate
    int to;         // ^ same
}thread_args;

struct fullTime{
    __clock_t timeR;
    __clock_t timeU;
    __clock_t timeS;
};

__clock_t mkTimeStamp(){

    struct timespec time_r; // tv_sec - full secs, tv_nsec - nsecs
    clock_gettime(CLOCK_REALTIME, &time_r);

    __clock_t realTime = time_r.tv_nsec + time_r.tv_sec * 1000000000;

    return  realTime;
}


int calculate_pixel(int x, int y, int W, int H, int C, int **I, float **K) {
    float s = 0;
    for (int j = 0; j < C; j++) {
        int b = (int) fmin(H-1,fmax(0, y - ceil(((float) C) / 2) + j));
 //       b = b < H ? b : H - 1;
        for (int i = 0; i < C; i++) {
            int a = (int) fmin(W-1,fmax(0, x - ceil(((float) C) / 2) + i));
   //         a = a < W ? a : W - 1;
            float e = I[b][a] * K[j][i];
            s += e;
        }
    }
    s = s < 0 ? 0 : s;
    return (int) round(s);
    if(s < 0) return 0;
    else return (int) round(s);
}


void *execute_task(void *args){
    thread_args *arguments = (thread_args *) args;
    for (int y = 0; y < arguments->H; y++)
        for (int x = arguments->from; x < arguments->to; x++) {
            arguments->result[y][x] = calculate_pixel(x, y, arguments->W, arguments->H, arguments->C, arguments->pgm, arguments->filter);
        }
    pthread_exit(NULL);
}


void save_filtered(int W, int H, int **picture, char *filename){
    //char buff[MAXBUF];
    FILE *file = fopen(filename,"w");
    if(file == NULL){
        printf("Couldn't open file %s!\n",filename);
        exit(EXIT_FAILURE);
    }

    fprintf(file, "P2\n");
    fprintf(file, "%d %d\n", W, H);
    fprintf(file, "%d\n", 255);
    for (int i = 0; i < H; i++) {
        for (int j = 0; j < W; j++) {
            fprintf(file, "%d ", picture[i][j]);
 /*           if (j < W - 1) {
                sprintf(buff, "%d ", picture[i][j]);
            } else {
                sprintf(buff, "%d\n", picture[i][j]);
            }
            fputs(buff, file); */
        }
        fprintf(file, "\n");
    }
    fclose(file);
}


int main(int argc, char **argv){

    if(argc != 5){
        printf("Give number of threads, filename to filter, filter and result file name!\n");
        return EXIT_FAILURE;
    }

    int threadnum = strtol(argv[1],NULL,10);
    if(threadnum < 1 || threadnum > 20){
        printf("Number of threads ranges between 1 and 20!\n");
        return EXIT_FAILURE;
    }

    FILE *pgmfile = fopen(argv[2], "r");
    if(pgmfile == NULL){
        printf("Couldn't open file %s!\n",argv[2]);
        return EXIT_FAILURE;
    }

    FILE *filterfile = fopen(argv[3],"r");
    if(filterfile == NULL){
        printf("Couldn't open file %s!\n",argv[3]);
        return EXIT_FAILURE;
    }


    char buff[MAXBUF];

    fgets(buff, MAXBUF, pgmfile);   // P2
    fgets(buff, MAXBUF, pgmfile);

    int W = (int) strtol(strtok(buff, " \t"),NULL,10);
    int H = (int) strtol(strtok(NULL, " \t\n\r"),NULL,10);

    int **pgm = calloc(H, sizeof(int*));
    int **result = calloc(H, sizeof(int*));


    for (int i = 0; i < H; i++) {
        pgm[i] = calloc(W, sizeof(int));
        result[i] = calloc(W, sizeof(int));
    }

    fgets(buff, MAXBUF, pgmfile);   // 255

    int row = 0;
    int column = 0;
    int flag;
    char* value;

    while(fgets(buff, MAXBUF, pgmfile) != NULL){        // LINE
        flag = 0;
        while((value = strtok(flag == 0 ? buff : NULL, " \t\n\r")) != NULL){  // PIXEL
            pgm[row][column] = (int) strtol(value, NULL, 10);
            column++;
            if (column == W) {
                row++;
                column = 0;
            }
            flag = 1;
        }
    }
    
    fclose(pgmfile);

    fgets(buff, MAXBUF, filterfile);
    int C = (int) strtol(strtok(buff, " \t\r\n"),NULL,10);

    float **filter = calloc(C, sizeof(float*));
    for (int i = 0; i < C; i++) {
        filter[i] = calloc(C, sizeof(float));
    }

    row = 0;
    column = 0;
    while(fgets(buff, MAXBUF, filterfile) != NULL){     // LINE
        flag = 0;
        while((value = strtok(flag == 0 ? buff : NULL, " \t\n\r")) != NULL){  // VALUE
            filter[row][column] = strtof(value, NULL);
            column++;
            if (column == C) {
                row++;
                column = 0;
            }
            flag = 1;
        }
    }

    fclose(filterfile);

    __clock_t stamp1 = mkTimeStamp();

    pthread_t *threads = calloc(threadnum, sizeof(pthread_t));                    // array of threads' ids

    thread_args **arguments = calloc(threadnum, sizeof(struct thread_args*));  // array of arguments to every thread

    for (int i = 0; i < threadnum; i++) {
        arguments[i] = malloc(sizeof(thread_args));
        arguments[i]->W = W;
        arguments[i]->H = H;
        arguments[i]->C = C;
        arguments[i]->pgm = pgm;
        arguments[i]->filter = filter;
        arguments[i]->result = result;
        arguments[i]->from = (i * W / threadnum);
        arguments[i]->to = ((i + 1) * W / threadnum);
    }
    arguments[threadnum-1]->to = W;           // last one gets more :(

    for (int i = 0; i < threadnum; i++) {
        pthread_create(&threads[i], NULL, &execute_task, (void *) arguments[i]);
    }

    for (int i = 0; i < threadnum; i++) {
        pthread_join(threads[i], NULL);
        free(arguments[i]);
    }
    free(arguments);

    __clock_t stamp2 = mkTimeStamp();

    printf("File size: %dx%d. Filter size: %d.\n\t Time needed to filter a picture: %f.\n\n",
            W, H, C, (float)((stamp2-stamp1)/1000000000.0));

    save_filtered(W,H,result,argv[4]);

    return EXIT_SUCCESS;
}