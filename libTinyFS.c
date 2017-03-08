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
   char *initializer;
   char setter;
   if ((fd = openDisk(filename, nBytes)) != 0)
   {
      /*ERROR THINGS*/
   } else {
      /*Initialze all data to 0x00*/
      numBlocks = (nBytes - (nBytes % BLOCKSIZE))/BLOCKSIZE;
      initializer = calloc(BLOCKSIZE, sizeof(char));/*set a blank block*/
      for (i = 0; i < numBlocks; i++) {
         writeBlock(fd, i, initializer);/*write blank data to every block*/
      }
      free(initializer);

      initializer = malloc(2*sizeof(char));
      initializer[0] = 0x04;/*free block code*/
      initializer[1] = 0x44;/*magic number*/
      for (i = 0; i < numBlocks; i++)
      {
         lseek(fd, BLOCKSIZE*i, 0);/*go to start of next block*/
         write(fd, initializer, 2*sizeof(char));/*write first 2 bytes*/
      } 
      
      /*initialize supernode, inodes, etc.*/
      lseek(fd, 0, 0);/*seek to start of superblock*/
      setter = 0x01; /*set setter as superblock code*/
      write(fd, &i, sizeof(char));/*write superblock code as 1 byte*/
      /*magic number 0x44 should already be in 2nd byte*/
      /*add the block number of the root inode*/
      /*a pointer to a list of free blocks (or another way to manage those)*/
      
      /*return exit code*/
   }
   return 0;
}
