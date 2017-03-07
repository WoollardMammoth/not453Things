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
int tfs_mkfs(char *filename, int nBytes){
   if (openDisk(filename, nBytes) != 0)
   {
      /*ERROR THINGS*/
   } else {
      /*Initialze all data to 0x00*/
      /*set magic numbers -- no idea what these are*/
      /*initialize supernode, inodes, etc.*/
      /*return exit code*/
   }
}
