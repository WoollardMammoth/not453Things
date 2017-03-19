#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "tinyFS.h"
#include "libTinyFS.h"
#include "libDisk.h"

#define TEST 1

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
   if(TEST){
      printf("TEST: The file  system '%s' was opened with fd '%d' and size of '%d' blocks\n", 
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


   if(TEST){
      printf("TEST: The disk that is now mounted is '%s'\n", diskname);
   }

   return 1; 
}

int tfs_unmount(void){

   if(mountedDisk == NULL){
      //throw file already unmounted error
   }

   if(TEST){
      printf("TEST: '%s' was unmounted\n", mountedDisk);
   }

   free(mountedDisk); 
   mountedDisk = NULL;

   //Clear DRT 
   if (resourceTable != NULL) {
      resourceTable = NULL;
   }

   if(TEST){
      printf("TEST: The dynamic resource table is now empty\n");
   }

   return 1;
}


/* Creates or Opens an existing file for reading and writing on the
currently mounted file system. Creates a dynamic resource table entry
for the file, and returns a file descriptor (integer) that can be
used to reference this file while the filesystem is mounted. */
fileDescriptor tfs_openFile(char *name){
	DRT *newDRT = NULL, 
      *tempDRT = resourceTable;
	fileDescriptor fd;
	int i;
	int present = 0;
	char buffer[BLOCKSIZE];
	char tempBuffer[BLOCKSIZE];
	int numBlocks = DEFAULT_DISK_SIZE / BLOCKSIZE;
	char nextFreeBlock = '\0';
   time_t cur;
   Inode newInode;

   if(TEST){
      printf("TEST: Attempting to open file '%s'\n", name);
   }

	if(mountedDisk == NULL){
		//return no mounted disk error
	}
	else{
		while(tempDRT != NULL){
			if(strcmp(tempDRT->filename,name) == 0){
				if(TEST){
               printf("TEST: '%s' was found in the Dynamic Resource Table. It has a FD of '%d'\n",
               name, tempDRT->fd);
            }
            return tempDRT->fd;
			}
			tempDRT = tempDRT->next;
		}
		fd = openDisk(mountedDisk, 0);
	}

	for(i = 0; i < numBlocks && !present; i++){
		if(readBlock(fd,i,buffer) < 0){
			/*Need to return Error*/
		}
      /* Check to see if it is an Inode */
		if(buffer[0] == 2){
			if(strcmp(name, &(buffer[4])) == 0){
				present = 1;
				if(TEST){
               printf("TEST: '%s' was found on the mounted disk\n");
            }
            break;
			}
		}
	}

   cur = time(NULL);

	if(!present){

      if(TEST){
         printf("TEST: '%s' was not found on the mounted disk\n", name);
      }

      /* Reading the Superblock */ 
		if(readBlock(fd, 0, buffer) < 0){
			/* return error */
		}
      /* Taking data out of the superblock */
		if(buffer[0] == 1){
			nextFreeBlock = buffer[3];
			readBlock(fd,nextFreeBlock,tempBuffer);
			buffer[3] = tempBuffer[2];
			writeBlock(fd,0,buffer);
		}
		else{
			/* return error for no super block */
		}

      if(TEST){
         printf("TEST: Creating a new Inode at location '%d' on the mounted disk\n", nextFreeBlock);
         printf("TEST: The new next free block is '%c'\n", tempBuffer[3]);
      }

      /*
      * This makes a new node at the location specified by nextFreeBlock 
      * and updates the root inode to point at this inode
      */
      newInode.blockType = 2;
      newInode.magicNum = 0x44;
      memcpy(newInode.name, name, 8);
      newInode.name[8] = '\0';
      newInode.startOfFile = -1;
      newInode.nextInode = buffer[2];
      buffer[2] = nextFreeBlock;
      writeBlock(fd,0,buffer);
      newInode.fp = 0;
      newInode.creationTime = cur;
      newInode.lastAccess = cur;

      /* This writes the new Inode to the disk */
      writeInode(fd,nextFreeBlock,&newInode);
       
	}
   else{
      if(TEST){
         printf("TEST: '%s' was found on the mounted disk\n", name);
      }
      /* Updates the inodes accesstime */
      newInode = readInode(fd,i);
      newInode.lastAccess = cur;
      writeInode(fd,i,&newInode);
   }

   /* Creating the new DRT entry for the table */ 

   if(TEST){
         printf("TEST: Creating a new DRT entry for '%s'\n", name);
      }

   newDRT = calloc(1, sizeof(DRT));
   strcpy(newDRT->filename, newInode.name);
   newDRT->creation = cur;
   newDRT->lastAccess = cur;

   /* Checks to see if the DRT table is empty, and assigns the correct
   * file descriptor */
   if(resourceTable == NULL){
      newDRT->fd = 1;
      newDRT->next = NULL;
   }
   else{
      newDRT->fd = resourceTable->fd + 1;
      newDRT->next = resourceTable;
      resourceTable = newDRT;
   }

   if(TEST){
      printf("TEST: '%s' was assigned FD '%d'\n", name, newDRT->fd);
   }

   close(fd);

	return tempDRT->fd;
}

/* Closes the file, de-allocates all system/disk resources, and
removes table entry */
int tfs_closeFile(fileDescriptor FD) {
   
   DRT *temp = resourceTable;
   DRT *previous;
   
   if(TEST){
      printf("TEST: Attempting to close file with FD '%d'\n", FD);
   }

   if(mountedDisk == NULL) {
      /*return not mounted error*/
   } else if (FD < 0) {
      /* return invalid FD error*/
   }
   
   if(temp != NULL && temp->fd == FD) {/*first entry is a match*/
      resourceTable = temp->next;
      free(temp);
      close(FD);
      if(TEST){
         printf("TEST: FD '%d' was closed\n", FD);
      }
      return 0;/*success*/
   } else {
      previous = temp;
      temp = temp->next;
      while (temp != NULL){
         if (temp->fd == FD) {
            previous->next = temp->next;
            free(temp);
            close(FD);
            if(TEST){
              printf("TEST: FD '%d' was closed\n", FD);
            }
            return 0;/*success*/
         }
         temp = temp->next;
      }
   }

   if(TEST){
      printf("TEST: Cannot close FD '%d,' it was not in open\n", FD);
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
   FreeBlock fb;
   DRT *temp = resourceTable;
   char inodeIdx = 0, curExtentIdx = 0, nextExtentIdx = 0, nextFree = 0;
   char *openFile;
   int mountedFD, extentDataSize = BLOCKSIZE - 3,
       numExtents, filePresent = 0, i;

   if(TEST){
      printf("TEST: Attempting to write '%d' bytes for the file with FD '%d'\n",
      size, FD);
   }       

   if (mountedDisk == NULL) {
      //No mounted disk error
   }
   if (FD < 0) { // Where is this coming from??
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
         openFile = calloc(sizeof(char), strlen(temp->filename) + 1);
         strcpy(openFile, temp->filename);
         filePresent = 1;
         if(TEST){
            printf("TEST: FD '%d' is an open file with name %s\n", fd. temp->filename);
         }
         break;
      }
      temp = temp->next;
   }
   
   if (!filePresent) {
      //Error file not open for writing
   }

   //Find inode for file
   sb = readSuperBlock(mountedFD);
   curInode = readInode(mountedFD, sb.rootInodeBlockNum);

   while (strcmp(curInode.name, openFile) != 0) {
      inodeIdx = curInode.nextInode;
      curInode = readInode(mountedFD, curInode.nextInode);
   }

   curInode.fp = 0;
   curInode.lastAccess = time(NULL);

   //writeInode(mountedFD, inodeIdx, &curInode);
   
   //Write buffer data
   if (size % 253 == 0) {
      numExtents = size/extentDataSize; 
   }
   else {
      numExtents = (size/extentDataSize) + 1;
   }


   //Delete current contents of file
   if (curInode.startOfFile != -1) {
      curExtentIdx = curInode.startOfFile;

      while (nextExtentIdx != -1) {
         nextExtentIdx = readFileExtent(mountedFD, curExtentIdx).nextBlock;

         fb.blockType = 4;
         fb.magicNum = 0x44;
         fb.nextFreeBlock = sb.freeBlocksRoot; 
         writeFreeBlock(mountedFD, curExtentIdx, &fb);
         
         sb.freeBlocksRoot = curExtentIdx;
         writeSuperBlock(mountedFD, &sb);

         curExtentIdx = nextExtentIdx;
      }

      curInode.startOfFile = readSuperBlock(mountedFD).freeBlocksRoot;
      writeInode(mountedFD, inodeIdx, &curInode);
   }

   //Write buffer data
   for (i = 0; i < numExtents; i++) {
      newExtent.blockType = 3;
      newExtent.magicNum = 0x44; 
 
      if (i == numExtents-1) { 
         newExtent.nextBlock = -1;
      }
      else { 
         newExtent.nextBlock =  readFreeBlock(mountedFD, sb.freeBlocksRoot).nextFreeBlock;
      }

      memcpy(buffer + (extentDataSize*i), newExtent.data, extentDataSize);

      nextFree = readFreeBlock(mountedFD, sb.freeBlocksRoot).nextFreeBlock;
      writeFileExtent(mountedFD, sb.freeBlocksRoot, &newExtent);

      sb.freeBlocksRoot = nextFree; 
      writeSuperBlock(mountedFD, &sb);
   }

   if(TEST){
      printf("TEST: Compled writing '%d' bytes of data to %s\n",
         size, temp->filename);
   } 

   return 0;
}

/* deletes a file and marks its blocks as free on disk. */
int tfs_deleteFile(fileDescriptor FD) {
   DRT *prev = NULL, 
      *temp = resourceTable;
   int mountedFD;
   SuperBlock sb;
   char targetInodeOffset;
   Inode in;
   FileExtent fe;
   FreeBlock fb;
   char fileExtentOffset;

   fb.blockType = 4;
   fb.magicNum = 0x44;

   if(TEST){
      printf("TEST: Attempting to delete the file with FD '%d'\n", FD);
   }

   if (FD < 0) {
      /*invalid FD error*/
   } else if (temp == NULL) {
      /*empty resource table error*/
   }
   
   while (temp != NULL) {
      if (temp->fd == FD) {
         if(TEST){
            printf("TEST: The file with FD '%d' was open and in the Dynamic Resource Table\n",
               FD);
         }
         prev->next = temp->next;
         break;
      }
      prev = temp;
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

   if(TEST){
      printf("TEST: '%s' with FD '%d' was closed and removed from the Dynamic Resource Table\n",
         temp->filename, temp->fd);
   }

   free(temp);

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

   if(TEST){
      printf("TEST: Attempting to read a byte from the file with FD '%d'\n", FD);
   }

   byteOffset = -1;
   currentByte = 0;
   if (FD < 0) {
      /*invalid FD error*/
   } else if (temp == NULL) {
      /*empty resource table error*/
   }
   
   while (temp != NULL) {
      if (temp->fd == FD) {
         if(TEST){
            printf("TEST: The file with FD '%d' was found in the Dynamic Resource Table\n",FD);
         }
         break;
      }
      temp = temp->next;
   }

   if (temp == NULL) {
      if(TEST){
         printf("TEST: The file with FD '%d' was not found in the Dynamic Resource Table\n", FD);
      }
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
   
   if (in.startOfFile == -1){
      /*no fileExtent data error*/
   }
   fe = readFileExtent(mountedFD, in.startOfFile);
 
   if (byteOffset % 253 == 0 && fe.nextBlock == -1){
      /*EOF error*/
   }  
   while(byteOffset - currentByte > 253) {
      if (fe.nextBlock == -1){
         /*pointed OOB error*/
      }
      readFileExtent(mountedFD, fe.nextBlock);
      currentByte+= 253;
   }
   /*the byte we want is in fe.data*/
   in.fp++;
   in.lastAccess = time(NULL);
   writeInode(mountedFD, inodeBlockNum, &in);

   memcpy(buffer, &(fe.data[byteOffset-currentByte]), 1); 

   if(TEST){
      printf("TEST: Successful reading of a byte from the file with FD '%d'\n", FD);
   }

   return 0;/*success*/
}

/* change the file pointer location to offset (absolute). Returns
success/error codes.*/
int tfs_seek(fileDescriptor FD, int offset){
   char *filename = NULL;
   DRT *cursor = resourceTable;
   int fd, i; 
   int numBlocks = DEFAULT_DISK_SIZE/BLOCKSIZE;
   time_t cur;
   Inode tempInode;

   if(TEST){
      printf("TEST: Attempting to seek to offset '%d' in the file with FD '%d'\n",
         offset, FD);
   }

   if(mountedDisk){
      fd = openDisk(mountedDisk,0);
   }
   else{
      /* return no file mounted file error */
   }

   while(cursor != NULL){
      if(cursor->fd == FD){
         filename = malloc(sizeof(char) * 9);
         strcpy(filename, cursor->filename);
         break;
      }
      cursor = cursor->next;
   }

   if(filename == NULL){
      /* return error */
   }

   for(i = 0; i < numBlocks; i++){
      tempInode = readInode(fd,i);
   
      if(tempInode.blockType == 2){
         if(!strcmp(filename, tempInode.name)){
            tempInode.fp = offset;
            cur = time(NULL);
            tempInode.lastAccess = cur;
            writeInode(fd,i,&tempInode);
            break;
         }
      }
   }
   return 1;
}


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
   strncpy(root.name, fname, 9);
   root.name[8] = '\0';
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
