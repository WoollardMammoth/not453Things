#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "tinyFS.h"
#include "libDisk.h"

disk diskArray[100];
int totalDisks;

int openDisk(char *filename, int nBytes) {
   int fd;
   disk newDisk;
   time_t currentTime;

   if(nBytes < BLOCKSIZE || nBytes < 0){
      /* Failure Returned */
      return -1;
   }
   else if(nBytes == 0){
      return getFD(filename);
   }
   else if(-1 == (fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0660))) {
      /* Error - problems with opening the file */
      return -1;
   }
 
   newDisk.fd = fd;
   newDisk.name = filename;
   /*nBytes is a multiple of 256, meaning it always ends on a block boundary*/
   newDisk.nBytes = nBytes - (nBytes % BLOCKSIZE);
   /*Should check to make sure that this is correct*/
   newDisk.data = malloc(newDisk.nBytes); 
   
   currentTime = time(NULL);
   newDisk.timeStamp = ctime(&currentTime);
   
   diskArray[totalDisks++] = newDisk;
   return fd;
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

int getFD(char *filename){
   int i;
   for(i = 0; i < totalDisks; i++){
      if(strcmp(filename, diskArray[i].name) == 0){
         return diskArray[i].fd;
      }
   }
   return -1;
}


