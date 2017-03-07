#ifndef LIBDISK_H
#define LIBDISK_H

typedef struct {
    int fd;
    void *data;
    int nBytes;
    char *timeStamp;
    char *name;
} disk;

int openDisk(char *filename, int nBytes);

int readBlock(int disk, int bNum, void *block);

int writeBlock(int disk, int bNum, void *block);
#endif
