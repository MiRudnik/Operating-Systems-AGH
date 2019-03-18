#ifndef CW01_BLOCKLIBRARY_H
#define CW01_BLOCKLIBRARY_H

#define MAXSIZE 10000

struct arrayS{
    int size;
    int length;
    char (*array)[MAXSIZE];
};

struct arrayD{
    int size;
    int length;
    char** array;
};

struct arrayS* createBlockArray_S(int size, int length);
struct arrayD* createBlockArray_D(int size, int length);
// creates static/dynamic array containing 'size' blocks of 'length' chars at most
void deleteBlockArray_S(struct arrayS* blockArray);
void deleteBlockArray_D(struct arrayD* blockArray);
// removes given array
void addBlock_S(struct arrayS* blockArray, char* block, int index);
void addBlock_D(struct arrayD* blockArray, char* block, int index);
// adds block of chars at given 'index' if space is not taken
void deleteBlock_S(struct arrayS* blockArray, int index);
void deleteBlock_D(struct arrayD* blockArray, int index);
// removes block at given 'index' if space is taken
int searchClosestAscii_S(struct arrayS* blockArray, int index);
int searchClosestAscii_D(struct arrayD* blockArray, int index);
// looks for block with sum of elements (in ASCII) closest to that of block at 'index'

#endif //CW01_BLOCKLIBRARY_H
