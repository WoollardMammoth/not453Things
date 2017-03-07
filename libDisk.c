#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "libTinyFS.h"
#include "tinyFS.h"

int openDisk(char *filename, int nBytes) {
   return 0;
}

int readBlock(int disk, int bNum, void *block) {
   unsigned byteOffset;
   byteOffset = bNum*BLOCKSIZE;
   
   /*skip disk to where we want*/
   if (-1 == lseek(disk, byteOffset, 0)) {
      /*lseek failed*/
      if (errno == EBADF) {
         /*ERROR -- file descriptor is not open*/
         return -1;
      }
      /*ERROR -- fd is open, but could not seek*/
      return -2;
   }
   if (read(disk, block, BLOCKSIZE) != BLOCKSIZE) {
      /*ERROR -- Could not read an entire block*/
      return -3;
   } else {
      /*SUCCESS*/
      return 0;
   } 
}

int writeBlock(int disk, int bNum, void *block) {
   return 0;
}
