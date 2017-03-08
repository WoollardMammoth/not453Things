#include "tinyFS.h"
#include "libDisk.h"

/*
 * Makes a blank TinyFS file system of size nBytes on the unix file specified
 * by ‘filename’. This function should use the emulated disk library to open
 * the specified unix file, and upon success, format the file to be mountable
 * disk. This includes initializing all data to 0x00, setting magic numbers,
 * initializing and writing the superblock and inodes, etc. Must return a
 * specified success/error code. 
 */
int tfs_mkfs(char *filename, int nBytes) {
   int i, fd, numBlocks;
   char *initilizer;
   char setter;
   if ((fd = openDisk(filename, nBytes)) != 0)
   {
      /*ERROR THINGS*/
   } else {
      /*Initialze all data to 0x00*/
      numBlocks = (nBytes - (nBytes % BLOCKSIZE))/BLOCKSIZE;
      initilizer = calloc(BLOCKSIZE, sizeof(char));/*set a blank block*/
      for (i = 0; i < numBlocks; i++) {
         writeBlock(fd, i, initilizer);/*write blank data to every block*/
      }

      /*set magic numbers -- 0x44 as the second byte of every block*/
      setter = 0x44;
      for (i = 0; i < numBlocks; i++)
      {
         lseek(fd, 1 + BLOCKSIZE*i, 0);/*go 1 byte passed start of next block*/
         write(fd, &setter, sizeof(char));/*put 0x44 at the 2nd byte*/
      } 
      
      /*initialize supernode, inodes, etc.*/
      /*return exit code*/
   }
   return 0;
}
