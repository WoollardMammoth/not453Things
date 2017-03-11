#include <stdlib.h>
#include <unistd.h>
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
      numBlocks = nBytes/BLOCKSIZE;/*by the magic of integer division*/
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
      free(initializer); 
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

/* tfs_mount(char *diskname) ​“mounts” a TinyFS file system located
within ‘diskname’ unix file. tfs_unmount(void) “unmounts” the
currently mounted file system. As part of the mount operation,
tfs_mount should verify the file system is the correct type. Only one
file system may be mounted at a time. Use tfs_unmount to cleanly
unmount the currently mounted file system. Must return a specified
success/error code. */
int tfs_mount(char *diskname);

int tfs_unmount(void);

/* Creates or Opens an existing file for reading and writing on the
currently mounted file system. Creates a dynamic resource table entry
for the file, and returns a file descriptor (integer) that can be
used to reference this file while the filesystem is mounted. */
fileDescriptor tfs_openFile(char *name);

/* Closes the file, de-allocates all system/disk resources, and
removes table entry */
int tfs_closeFile(fileDescriptor FD);

/* Writes buffer ‘buffer’ of size ‘size’, which represents an entire
file’s content, to the file system. Previous content (if any) will be
completely lost. Sets the file pointer to 0 (the start of file) when
done. Returns success/error codes. */
int tfs_writeFile(fileDescriptor FD,char *buffer, int size);

/* deletes a file and marks its blocks as free on disk. */
int tfs_deleteFile(fileDescriptor FD);

/* reads one byte from the file and copies it to buffer, using the
current file pointer location and incrementing it by one upon
success. If the file pointer is already at the end of the file then
tfs_readByte() should return an error and not increment the file
pointer. */
int tfs_readByte(fileDescriptor FD, char *buffer);

/* change the file pointer location to offset (absolute). Returns
success/error codes.*/
int tfs_seek(fileDescriptor FD, int offset);

