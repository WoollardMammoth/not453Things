#ifndef LIBDISK_H
#define LIBDISK_H

#include <time.h>

typedef struct {
    int fd;
    int nBlocks;
    time_t timeStamp;
    char *name;
} disk;

/*-----------------MANDATORY METHODS-----------------*/
int openDisk(char *filename, int nBytes);

int readBlock(int disk, int bNum, void *block);

int writeBlock(int disk, int bNum, void *block);
/*---------------------------------------------------*/


/*----------------SUPPORT METHODS--------------------*/
int getFD(char *filename);
/*---------------------------------------------------*/
#endif
