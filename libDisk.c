#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include "tinyFS.h"
#include "libDisk.h"

int openDisk(char *filename, int nBytes) {
   return 0;
}

int readBlock(int disk, int bNum, void *block) {
   unsigned byteOffset;
<<<<<<< HEAD

   char *wasteThisData;
   
=======
>>>>>>> 2e505a485a9485f142802bf7d33cd2026a0ba89a
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
   
   if (lseek(disk, bNum*BLOCKSIZE, 0) == -1) {
      /*lseek failed*/
      if (errno == EBADF) {
         /*ERROR -- file descriptor is not open*/
         return -1;
      }
      else {
         /*ERROR -- seek failed*/
         return -2;
      }
   }
  
   if (write(disk, block, BLOCKSIZE) != BLOCKSIZE) {
      /*ERROR -- Could not write entire block*/
      return -3;
   }

   return 0;
}

