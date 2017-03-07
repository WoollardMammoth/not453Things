#ifndef LIBDISK_H
#define LIBDISK_H
int openDisk(char *filename, int nBytes);
/*another test*/
int readBlock(int disk, int bNum, void *block);

int writeBlock(int disk, int bNum, void *block);
#endif
