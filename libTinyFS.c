#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "tinyFS.h"
#include "libTinyFS.h"
#include "libDisk.h"

#define DEBUG 0

static char *mountedDisk = NULL; // This is the name of the disk that is mounted
static DRT *resourceTable = NULL;

/*
 * Makes a blank TinyFS file system of size nBytes on the unix file specified
 * by ‘filename’. This function should use the emulated disk library to open
 * the specified unix file, and upon success, format the file to be mountable
 * disk. This includes initializing all data to 0x00, setting magic numbers,
 * initializing and writing the superblock and inodes, etc. Must return a
 * specified success/error code. 
 */
int tfs_mkfs(char *filename, int nBytes) {
   int fd, numBlocks;
   if ((fd = openDisk(filename, nBytes)) != 0)
   {
      /*ERROR THINGS*/
   } else {
      numBlocks = nBytes/BLOCKSIZE;/*by the magic of integer division*/
      return setUpFS(fd, filename, numBlocks);
   }
   if(DEBUG){
      printf("DEBUG: The file  system %s was opened with fd '%d' and size of '%d' blocks\n", 
         filename, fd, numBlocks);
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
int tfs_mount(char *diskname){

   char buff[BLOCKSIZE];
   fileDescriptor fd;
   int readStatus;



   if(mountedDisk){
      tfs_unmount();
   }

   if(0 > (fd = openDisk(diskname, 0))){
      //Throw bad fs error
   }

   if(0 > (readStatus = readBlock(fd, 0, buff))){
      // Throw error that it could not read the disk
   }

   if(buff[1] != 0x44){
      //Throw error that ut is a bad fs
   }

   
   //mountedDisk = diskname;
   mountedDisk = calloc(sizeof(char), strlen(diskname) + 1);
   strcpy(mountedDisk, diskname);


   if(DEBUG){
      printf("DEBUG: The disk that is now mounted is %s\n", diskname);
   }

   return 1; 
}

int tfs_unmount(void){

   if(mountedDisk){
      //throw file already unmounted error
   }

   if(DEBUG){
      printf("DEBUG: %s was unmounted\n", mountedDisk);
   }

   free(mountedDisk); 
   mountedDisk = NULL;

   return 1;
}

/* Creates or Opens an existing file for reading and writing on the
currently mounted file system. Creates a dynamic resource table entry
for the file, and returns a file descriptor (integer) that can be
used to reference this file while the filesystem is mounted. */
fileDescriptor tfs_openFile(char *name){
	DRT *temp = resourceTable;
	fileDescriptor fd;
	int i;
	int present = 0;
	char readBuffer[BLOCKSIZE];

	if(mountedDisk == NULL){
		//return no mounted disk error
	}
	else{
		while(temp != NULL){
			if(strcmp(temp->filename,name) == 0){
				return temp->fd;
			}
			temp = temp->next;
		}
		fd = openDisk(mountedDisk, 0);
	}

	for(i = 0; i < DEFAULT_DISK_SIZE/BLOCKSIZE && !present; i++){
		if(readBlock(fd,i,readBuffer) < 0){
			/*Need to return Error*/
		}
		if(readBuffer[0] == 2){
			if(strcmp(name, &(readBuffer[4])) == 0){
				present = 1;
				break;
			}
		}
	}
	return fd;
}

/* Closes the file, de-allocates all system/disk resources, and
removes table entry */
int tfs_closeFile(fileDescriptor FD) {
   
   DRT *temp = resourceTable;
   DRT *previous;
   if(mountedDisk == NULL) {
      /*return not mounted error*/
   } else if (FD < 0) {
      /* return invalid FD error*/
   }
   
   if(temp != NULL && temp->fd == FD) {/*first entry is a match*/
      resourceTable = temp->next;
      free(temp);
      close(FD);
      return 0;/*success*/
   } else {
      previous = temp;
      temp = temp->next;
      while (temp != NULL){
         if (temp->fd == FD) {
            previous->next = temp->next;
            free(temp);
            close(FD);
            return 0;/*success*/
         }
         temp = temp->next;
      }
   }
   return -1; /*FD not here/valid*/
}

/* Writes buffer ‘buffer’ of size ‘size’, which represents an entire
file’s content, to the file system. Previous content (if any) will be
completely lost. Sets the file pointer to 0 (the start of file) when
done. Returns success/error codes. */
int tfs_writeFile(fileDescriptor FD,char *buffer, int size) {
   Inode newInode;
   
   DRT *temp = resourceTable;
   DRT *previous;


   if (mountedDisk == NULL) {
      //No mounted disk error
   }
   if (FD < 0) {
      //Invalid FD error
   }

   //Create new inode
   newInode.blocktype = 1;
   newInode.magicNum = 0x44;

   //Traverse data table to find file name
   if (temp == NULL) {
      //Error? resource table empty
   }

   while (temp != NULL) {
      if (temp->fd == FD) {
         strcpy(newInode.name, temp->filename);
         break;
      }
      temp = temp->next;
   }

   /*****Atempted to simplify this: see above***
   if(temp != NULL && temp->fd == FD) {
      strcpy(newInode.name, temp->filename;
   }
   else {
      temp = temp->next; 
      while (temp != NULL) {
         if (temp->fd == FD) {
            strcpy(newInode.name, temp->filename);
            break;
         }
         temp = temp->next;
      }
   }
   */


   //Append inode to list 
   //Point start of inode file to freeblocksroot from superblock 
   //Write contents of buffer to file extents

   return 0;
}

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


int setUpFS(int fd, char *fname, int nBlocks) {
   SuperBlock sb;
   Inode root;
   FreeBlock everythingElse;
   int i;
   char *initializer;

   sb.blockType = 1;
   sb.magicNum = 0x44;
   sb.rootInodeBlockNum = 1;
   sb.freeBlocksRoot = 2;
   
   root.blockType = 2;
   root.magicNum = 0x44;
   memcpy(root.name, fname, 9);
   /*timestamp things*/

   everythingElse.blockType = 4;
   everythingElse.magicNum = 0x44;

   initializer = calloc(BLOCKSIZE, sizeof(char));/*set a blank block*/
   for (i = 0; i < nBlocks; i++) {
      if(0 != writeBlock(fd, i, initializer)) {/*write blank data to every block*/
         return -10;/*initilizing data to 0 failed*/
      }
   }
   free(initializer);

   if (0 != writeBlock(fd, 0, &sb)) {
      return -11; /*writing superBlock failed*/
   }
   if (0 != writeBlock(fd, 1, &root)) {
      return -12; /*writing root inode failed*/      
   }

   for (i = 2; i<nBlocks; i++) {
      everythingElse.nextFreeBlock = i+1;
      if (i+1 == nBlocks) {
         everythingElse.nextFreeBlock = -1;
      }
      if (0 != writeBlock(fd, i, &everythingElse)) {
         return -13; /*writing free block failed*/
      }
   }
   return 0; /*success*/
}


