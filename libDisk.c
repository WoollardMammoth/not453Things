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
#include "libTinyFS.h"

int openDisk(char *filename, int nBytes) {
   int fd;

   if((nBytes == 0) && (access(filename, F_OK) != -1)){	
   	fd = open(filename, O_RDWR, 0660);
      return fd;
   }
   else if(nBytes < BLOCKSIZE) {
	   /* Failure Returned */
    	return -1;
   }
   else if(-1 == (fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0660))) {
      /* Error - problems with opening the file */
      return -1;
   }

   return fd;
}

int readBlock(int disk, int bNum, void *block) {
   unsigned byteOffset;
   
   byteOffset = bNum*BLOCKSIZE;
   
   /*skip disk to where we want*/
   if (-1 == lseek(disk, byteOffset, 0)) {
      if(TEST){
         printf("TEST: Lseek has failed in readBlock\n");
      }
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
      return -4;
   }

   return 0;
}


