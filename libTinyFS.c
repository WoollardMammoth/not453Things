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

/* tfs_mount(char *diskname) “mounts” a TinyFS file system located
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

   if(mountedDisk == NULL){
      //throw file already unmounted error
   }

   if(DEBUG){
      printf("DEBUG: %s was unmounted\n", mountedDisk);
   }

   free(mountedDisk); 
   mountedDisk = NULL;

   //Clear DRT 
   if (resourceTable != NULL) {
      resourceTable = NULL;
   }

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
	char buffer[BLOCKSIZE];
	char tempBuffer[BLOCKSIZE];
	time_t currentTime;
	int numBlocks = DEFAULT_DISK_SIZE / BLOCKSIZE;
	char nextFreeBlock = '\0';

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

	for(i = 0; i < numBlocks && !present; i++){
		if(readBlock(fd,i,buffer) < 0){
			/*Need to return Error*/
		}
      if(buffer[0] == 2){
			if(strcmp(name, &(buffer[4])) == 0){
				present = 1;
				break;
			}
		}
	}

	currentTime = time(NULL);

	if(!present){
		if(readBlock(fd, 0, buffer) < 0){
			/* return error */
		}
		if(buffer[0] == 1){
			nextFreeBlock = buffer[3];
			readBlock(fd,nextFreeBlock,tempBuffer);
			buffer[3] = tempBuffer[2];
			writeBlock(fd,0,buffer);
		}
		else{
			/* return error for no super node */
		}
	}

	if(nextFreeBlock < 0)
	return fd;
   return -1;
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
int tfs_writeFile(fileDescriptor FD, char *buffer, int size) {
   Inode curInode;
   FileExtent newExtent; 
   SuperBlock sb;  
   DRT *temp = resourceTable;
   char inodeIdx, extentIdx;
   char *openFile;
   int mountedFD;
   int extentDataSize = BLOCKSIZE - 3,
       numExtents, filePresent = 0,
       nextFree,
       i;

   if (mountedDisk == NULL) {
      //No mounted disk error
   }
   if (FD < 0) {
      //Invalid FD error
   }

   mountedFD = openDisk(mountedDisk, 0);

   if (temp == NULL) {
      //Resource table empty - ERROR?
      //File is not open for writing 
   }

   //Check DRT for open file
   while (temp != NULL) {
      if (temp->fd == FD) {
         openFile = calloc(sizeof(char), strlen(temp->fileName) + 1);
         strcpy(openFile, temp->fileName);
         //strcpy(newInode.name, temp->filename);
         filePresent = 1;
         break;
      }
      temp = temp->next;
   }
   
   if (!filePresent) {
      //Error file not open for writing
   }

   //Find inode for file
   sb = readSuperBlock(mountedFD);
   inodeIdx = sb.rootInodeBlockNum;
   curInode = readInode(mountedFD, sb.rootInodeBlockNum);

   while (strcmp(curInode.name, openFile) != 0) {
      inodeIdx = curInode.nextInode;
      curInode = readInode(mountedFd, curInode.nextInode);
   }

   curInode.fp = 0;
   curInode.lastAccess = time(NULL);

   writeInode(mountedFD, inodeIdx, &curInode);
   
   //Write buffer data
   if (size % 253 == 0) {
      numExtents = size/extentDataSize; 
   }
   else {
      numExtents = (size/extentDataSize) + 1;
   }

   extentIdx = curInode.startOfFile;

   for (i = 0; i < numExtents; i++) {
      newExtent.blockType = 3;
      newExtent.magicNum = 0x44; 
   
      if (i == numExtents-1) { 
         newExtent.nextBlock = -1;
      }
      else {
         newExtent.nextBlock = readFileExtent(mountedFD, extentIdx).nextBlock; 
      }

      memcpy(buffer + (extentDataSize*i), newExtent.data, extentDataSize);

      writeFileExtent(mountedFD, extentIdx, &newExtent);
      extentIdx = newExtent.nextBlock;
   } 

   return 0;
}

/* deletes a file and marks its blocks as free on disk. */
int tfs_deleteFile(fileDescriptor FD) {
   DRT *temp = resourceTable;
   int mountedFD;
   SuperBlock sb;
   char targetInodeOffset;
   Inode ind231;
   FileExtent fe;
   FreeBlock fb;
   char fileExtentOffset;

   fb.blockType = 4;
   fb.magicNum = 0x44;

   if (FD < 0) {
      /*invalid FD error*/
   } else if (temp == NULL) {
      /*empty resource table error*/
   }
   
   while (temp != NULL) {
      if (temp->fd == FD) {
         break;
      }
      temp = temp->next;
   }

   if (temp == NULL) {
      /*FD not open error*/
   }
   mountedFD = openDisk(mountedDisk, 0);
   sb = readSuperBlock(mountedFD);
   in = readInode(mountedFD, sb.rootInodeBlockNum);

   while (strcmp(in.name, temp->filename) != 0) {
      if (in.nextInode == -1) {
         /*filename not found error*/
      }
      targetInodeOffset = in.nextInode;
      in = readInode(mountedFD, in.nextInode);
   }

   /*in is the Inode of the file with file descriptor FD*/
   fileExtentOffset = in.startOfFile;
   fb.nextFreeBlock = sb.freeBlocksRoot;
   sb.freeBlocksRoot = targetInodeOffset;
   writeFreeBlock(mountedFD, sb.freeBlocksRoot, &fb);
   while(fileExtentOffset != -1) {
      fe = readFileExtent(mountedFD, fileExtentOffset);
      fb.nextFreeBlock = sb.freeBlocksRoot;
      sb.freeBlocksRoot = fileExtentOffset;
      writeFreeBlock(mountedFD, sb.freeBlocksRoot, &fb);
      fileExtentOffset = fe.nextBlock;
   }
   writeSuperBlock(mountedFD, &sb);
   return 0;
}

/* reads one byte from the file and copies it to buffer, using the
current file pointer location and incrementing it by one upon
success. If the file pointer is already at the end of the file then
tfs_readByte() should return an error and not increment the file
pointer. */
int tfs_readByte(fileDescriptor FD, char *buffer) {
   DRT *temp = resourceTable;
   int mountedFD;
   SuperBlock sb;
   Inode in;
   FileExtent fe;
   char inodeBlockNum;
   int byteOffset;
   int currentByte;

   byteOffset = -1;
   currentByte = 0;
   if (FD < 0) {
      /*invalid FD error*/
   } else if (temp == NULL) {
      /*empty resource table error*/
   }
   
   while (temp != NULL) {
      if (temp->fd == FD) {
         break;
      }
      temp = temp->next;
   }

   if (temp == NULL) {
      /*FD not open error*/
   }
   mountedFD = openDisk(mountedDisk, 0);

   sb = readSuperBlock(mountedFD);
   in = readInode(mountedFD, sb.rootInodeBlockNum);

   while (strcmp(in.name, temp->filename) != 0) {
      if (in.nextInode == -1) {
         /*filename not found error*/
      }
      inodeBlockNum = in.nextInode;
      in = readInode(mountedFD, in.nextInode);
   }

   byteOffset = in.fp;
   in.fp++;
   writeInode(mountedFD, inodeBlockNum, &in);

   if (in.startOfFile == -1){
      /*no fileExtent data error*/
   }
   fe = readFileExtent(mountedFD, in.startOfFile);
   while(byteOffset - currentByte > 253) {
      if (fe.nextBlock == -1){
         /*pointed OOB error*/
      }
      readFileExtent(mountedFD, fe.nextBlock);
      currentByte+= 253;
   }
   /*the byte we want is in fe.data*/
   memcpy(buffer, &(fe.data[byteOffset-currentByte]), 1); 
   return 0;/*success*/
}

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
   root.creationTime = time(NULL);
   root.lastAccess = time(NULL);
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


/*----------Method stubs for read/write structs----------*/
SuperBlock readSuperBlock(fileDescriptor fd){ 
   SuperBlock sb;
   readBlock(fd, 0, &sb);
   return sb;
}
int writeSuperBlock(fileDescriptor fd, SuperBlock *sb) {
   return writeBlock(fd, 0, sb);
}

Inode readInode(fileDescriptor fd, char blockNum) {
   Inode in;
   readBlock(fd, blockNum, &in);
   return in;
}

int writeInode(fileDescriptor fd, char blockNum, Inode *in) {
   return writeBlock(fd, blockNum, in);
}

FileExtent readFileExtent(fileDescriptor fd, char blockNum) {
   FileExtent fe;
   readBlock(fd, blockNum, &fe);
   return fe;
}

int writeFileExtent(fileDescriptor fd, char blockNum, FileExtent *fe) {
   return writeBlock(fd, blockNum, fe);
}

int writeFreeBlock(fileDescriptor fd, char blockNum, FreeBlock *fb) {
   return writeBlock(fd, blockNum, fb);
}

FreeBlock readFreeBlock(fileDescriptor fd, char blockNum) {
   FreeBlock fb;
   readBlock(fd, blockNum, &fb);
   return fb;
}
