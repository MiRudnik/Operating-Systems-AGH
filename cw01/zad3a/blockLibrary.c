#include "blockLibrary.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

char staticArray[MAXSIZE][MAXSIZE]; // made to define static array

struct arrayS* createBlockArray_S(int size, int length){
    struct arrayS *newArray = malloc(sizeof(struct arrayS));
    newArray->size = size;
    newArray->length = length;
    newArray->array = staticArray;
    return newArray;
}

struct arrayD* createBlockArray_D(int size, int length){
    struct arrayD* newArray = malloc(sizeof(struct arrayD));
    newArray->size = size;
    newArray->length = length;
    newArray->array = (char**) calloc(size, sizeof(char*));
    return newArray;
}


void deleteBlockArray_S(struct arrayS* blockArray){
        for(int i=0;i<blockArray->size;i++){
            for(int j=0;j<blockArray->length;j++) {
                blockArray->array[i][j] = '\0';
            }
        }
    free(blockArray);
}

void deleteBlockArray_D(struct arrayD* blockArray){
    for(int i=0;i<blockArray->size;i++){
        if(blockArray->array != NULL) free(blockArray->array[i]);
    }
    free(blockArray);
}

void addBlock_S(struct arrayS* blockArray, char* block, int index){
    if(index < 0 || index > blockArray->size){
        printf("Wrong index.\n");
        return;
    }
    if(strlen(block) >= blockArray->length)
        printf("String too long, it will be cut down to %d characters.\n", blockArray->length-1);
    strncpy(blockArray->array[index], block, blockArray->length);   // strncpy adds \0 into empty spaces
    blockArray->array[index][blockArray->length - 1] = '\0';        // to ensure that string is terminated
}

void addBlock_D(struct arrayD* blockArray, char* block, int index){
    if(index < 0 || index > blockArray->size){
        printf("Wrong index.\n");
        return;
    }
    blockArray->array[index] = calloc(blockArray->length, sizeof(char));
    if(strlen(block) >= blockArray->length)
        printf("String too long, it will be cut down to %d characters.\n", blockArray->length-1);
    strncpy(blockArray->array[index], block, blockArray->length);   // strncpy adds \0 into empty spaces
    blockArray->array[index][blockArray->length - 1] = '\0';        // to ensure that string is terminated
}

void deleteBlock_S(struct arrayS* blockArray, int index){
    if(index < 0 || index > blockArray->size){
        printf("Wrong index.\n");
        return;
    }
    if(blockArray->array[index] == NULL){
        //printf("There is no block at given index.\n");
        return;
    }
    for(int i=0;i<blockArray->length;i++){
            blockArray->array[index][i] = '\0';
    }
}

void deleteBlock_D(struct arrayD* blockArray, int index){
    if(index < 0 || index > blockArray->size){
        printf("Wrong index.\n");
        return;
    }
    if(blockArray->array[index] == NULL){
        //printf("There is no block at given index.\n");
        return;
    }
        free(blockArray->array[index]);
        blockArray->array[index] = NULL;
}

int asciiToInt(char* block){        // returns sum of characters
    int sum = 0;
    int l = strlen(block);
    for(int i=0;i<l;i++){
        sum += (int) block[i];
    }
    return sum;
}


int searchClosestAscii_S(struct arrayS* blockArray, int index){
    int closest = index;
    int indexSum = asciiToInt(blockArray->array[index]);
    int smallestDifference = INT_MAX;
    if(index < 0 || index > blockArray->size){
        printf("Wrong index.\n");
        return -1;
    }
    if(blockArray->array[index] == NULL){
        printf("There is no block at given index.\n");
        return -1;
    }
    for(int i=0;i<blockArray->size;i++){
        char* string = blockArray->array[i];
        if(string != NULL && i != index){
            int difference = abs(asciiToInt(blockArray->array[i]) - indexSum);
            if(difference <= smallestDifference){
                closest = i;
                smallestDifference = difference;
            }
        }
    }
    if(closest == index) {
        printf("There is only one block in the array.\n");
        return -1;
    }
    else
        return closest;
}

int searchClosestAscii_D(struct arrayD* blockArray, int index){
    int closest = index;
    int indexSum = asciiToInt(blockArray->array[index]);
    int smallestDifference = INT_MAX;
    if(index < 0 || index > blockArray->size){
        printf("Wrong index.\n");
        return -1;
    }
    if(blockArray->array[index] == NULL){
        printf("There is no block at given index.\n");
        return -1;
    }
    for(int i=0;i<blockArray->size;i++){
        char* string = blockArray->array[i];
        if(string != NULL && i != index){
            int difference = abs(asciiToInt(blockArray->array[i]) - indexSum);
            if(difference <= smallestDifference){
                closest = i;
                smallestDifference = difference;
            }
        }
    }
    if(closest == index) {
        printf("There is only one block in the array.\n");
        return -1;
    }
    else
        return closest;
}
