#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "tinyFS.h"
#include "libTinyFS.h"
#include "libDisk.h"
#include "TinyFS_errno.h"


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
   int fd, numBlocks = 0;
   if ((fd = openDisk(filename, nBytes)) < 0)
   {
      return ERR_BADFD;
   } else {
      numBlocks = nBytes/BLOCKSIZE;/*by the magic of integer division*/
      return setUpFS(fd, filename, numBlocks);
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

   if(TEST){
      printf("TEST: Attempting to mount disk '%s'\n", diskname);
   }

   if(mountedDisk){
      if(TEST){
         printf("TEST: There is a disk already mounted. Unmounting '%s'\n", mountedDisk);
      }
      tfs_unmount();
   }

   if(0 > (fd = openDisk(diskname, 0))){
      return ERR_BADFS;
   }

   if(0 > (readStatus = readBlock(fd, 0, buff))){
      return ERR_READDISK; // Throw error that it could not read the disk
   }

   if(buff[1] != 0x44){
      return ERR_BADMAGICNUM;
   }

   
   mountedDisk = calloc(sizeof(char), strlen(diskname) + 1);
   strcpy(mountedDisk, diskname);


   if(TEST){
      printf("TEST: The disk that is now mounted is '%s'\n", diskname);
   }

   return 1; 
}

int tfs_unmount(void){

   if(mountedDisk == NULL){
      if(TEST){
         printf("TEST: There is no disk to unmount\n");
      }
      return ERR_NOMOUNTEDDISK;
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
	DRT *newDRT, 
      *tempDRT = resourceTable;
	fileDescriptor fd;
	int i;
	int present = 0;
	char tempBuffer[BLOCKSIZE];
	int numBlocks = DEFAULT_DISK_SIZE / BLOCKSIZE;
	char nextFreeBlock = -1;
   time_t cur;
   Inode newInode, tempInode;
   SuperBlock superBlock;

   if(TEST){
      printf("TEST: Attempting to open file '%s'\n", name);
   }

	if(mountedDisk == NULL){
      if(TEST){
         printf("TEST: There is no mounted disk\n");
      }
		return ERR_NOMOUNTEDDISK;
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

   if(TEST){
      printf("TEST: The FD for the mounted disk is '%d'\n", fd);
   }

	for(i = 0; i < numBlocks && !present; i++){
		
      if((tempInode = readInode(fd, i)).blockType < 0){
         printf("%d\n", i);
         if(TEST){
            printf("TEST: There is no more space on the disk\n");
         }
			return ERR_NOSPACE;
		}
      /* Check to see if it is an Inode */
		if(tempInode.blockType == 2){
			if(strcmp(name, tempInode.name) == 0){
				present = 1;
				if(TEST){
               printf("TEST: '%s' was found on the mounted disk\n", name);
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
		if((superBlock = readSuperBlock(fd)).blockType < 0){
			return ERR_READDISK;
		}
      /* Taking data out of the superblock */
		if(superBlock.blockType == 1){
			nextFreeBlock = superBlock.freeBlocksRoot;
			readBlock(fd,nextFreeBlock,tempBuffer);
			superBlock.freeBlocksRoot = tempBuffer[2];
			if (writeSuperBlock(fd,&superBlock) < 0) {
            return ERR_WRITEDISK;
         }
		}
		else{
			return ERR_BADFS;
		}

      if(TEST){
         printf("TEST: Creating a new Inode at location '%d' on the mounted disk\n", nextFreeBlock);
         printf("TEST: The new next free block is '%d'\n", superBlock.freeBlocksRoot);
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
      newInode.nextInode = superBlock.rootInodeBlockNum;
      superBlock.rootInodeBlockNum = nextFreeBlock;
      writeBlock(fd,0,&superBlock);
      newInode.fp = 0;
      newInode.creationTime = cur;
      newInode.lastAccess = cur;

      /* This writes the new Inode to the disk */
      if (nextFreeBlock == -1) {
         return ERR_NOSPACE;
      }
      if (writeInode(fd,nextFreeBlock,&newInode) < 0) {
         return ERR_WRITEDISK;
      }
       
	}
   else{
      if(TEST){
         printf("TEST: '%s' was found on the mounted disk\n", name);
      }
      /* Updates the inodes accesstime */
      if ((newInode = readInode(fd,i)).blockType < 0) {
         return ERR_READDISK;
      }

      newInode.lastAccess = cur;
      if (writeInode(fd,i,&newInode) < 0) {
         return ERR_WRITEDISK;
      }
   }

   /* Creating the new DRT entry for the table */ 

   if(TEST){
         printf("TEST: Creating a new DRT entry for '%s'\n", name);
   }

   newDRT = calloc(1,sizeof(DRT));

   strncpy(newDRT->filename, name, 8);
   newDRT->filename[8] = '\0';
   newDRT->creation = cur;
   newDRT->lastAccess = cur;


   /* Checks to see if the DRT table is empty, and assigns the correct
   * file descriptor */
   if(resourceTable == NULL){
      newDRT->fd = fd + 1;
   }
   else{
      newDRT->fd = resourceTable->fd + 1;
   }
   newDRT->next = resourceTable;
   resourceTable = newDRT;
   
   if(TEST){
      printf("TEST: '%s' was assigned FD '%d'\n", name, newDRT->fd);
   }

   close(fd);

	return newDRT->fd;
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
      return ERR_FILENOTOPEN;
   } else if (FD < 0) {
      return ERR_BADFD;
   }
   
   if(temp != NULL && temp->fd == FD) {/*first entry is a match*/
      resourceTable = temp->next;
      free(temp);
      if(TEST){
         printf("TEST: FD '%d' was closed\n", FD);
      }
      return 0;/*success*/
   } else {
      previous = temp;
      while (temp != NULL){
         if (temp->fd == FD) {
            previous->next = temp->next;
            free(temp);
            if(TEST){
              printf("TEST: FD '%d' was closed\n", FD);
            }
            return 0;/*success*/
         }
         previous = temp;
         temp = temp->next;
      }
   }

   if(TEST){
      printf("TEST: Cannot close FD '%d,' it was not open\n", FD);
   }
   return ERR_FILENOTOPEN; /*FD not here/valid*/
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
      return ERR_NOMOUNTEDDISK;
   }
   if (FD < 0) { 
      return ERR_BADFD;
   }

   mountedFD = openDisk(mountedDisk, 0);

   if (temp == NULL) {
      return ERR_BADFILE; 
   }

   //Check DRT for open file
   while (temp != NULL) {
      if (temp->fd == FD) {
         openFile = calloc(sizeof(char), strlen(temp->filename) + 1);
         strcpy(openFile, temp->filename);
         filePresent = 1;
         if(TEST){
            printf("TEST: FD '%d' is an open file with name %s\n", FD, temp->filename);
         }
         break;
      }
      temp = temp->next;
   }
  



   if (!filePresent) {
      return ERR_BADFILE;
   }

   //Find inode for file
   if((sb = readSuperBlock(mountedFD)).blockType < 0) {
      return ERR_READDISK;
   }
   if ((curInode = readInode(mountedFD, sb.rootInodeBlockNum)).blockType < 0 ) {
      return ERR_READDISK;
   }

   while (strcmp(curInode.name, openFile) != 0) {
      inodeIdx = curInode.nextInode;
      if ((curInode = readInode(mountedFD, curInode.nextInode)).blockType < 0) {
         return ERR_READDISK;
      }
   }
   curInode.fp = 0;
   curInode.lastAccess = time(NULL);



   //Delete current contents of file
   if (curInode.startOfFile != -1) {
      curExtentIdx = curInode.startOfFile;

      while (nextExtentIdx != -1) {
         if ((newExtent = readFileExtent(mountedFD, curExtentIdx)).blockType < 0) {
            return ERR_READDISK;
         }

         nextExtentIdx = newExtent.nextBlock;

         fb.blockType = 4;
         fb.magicNum = 0x44;
         fb.nextFreeBlock = sb.freeBlocksRoot; 
         if (writeFreeBlock(mountedFD, curExtentIdx, &fb) < 0) {
            return ERR_WRITEDISK;
         }
         
         sb.freeBlocksRoot = curExtentIdx;
         if (writeSuperBlock(mountedFD, &sb) < 0) {
            return ERR_WRITEDISK;
         }

         curExtentIdx = nextExtentIdx;
      }

      curInode.startOfFile = sb.freeBlocksRoot;
      if (writeInode(mountedFD, inodeIdx, &curInode) < 0) {
         return ERR_WRITEDISK;
      }
   }
 
   if (size % 253 == 0) {
      numExtents = size/extentDataSize; 
   }
   else {
      numExtents = (size/extentDataSize) + 1;
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

         if (newExtent.blockType < 0) {
            return ERR_READDISK;
         }
      }
      
      if (size < BLOCKSIZE - 3) {
         extentDataSize = size;
      }

      memcpy(newExtent.data, buffer + (extentDataSize*i), extentDataSize);

      if ((fb = readFreeBlock(mountedFD, sb.freeBlocksRoot)).blockType < 0) {
         return ERR_READDISK;
      }
      
      nextFree = fb.nextFreeBlock;
      if (writeFileExtent(mountedFD, sb.freeBlocksRoot, &newExtent) < 0) {
         return ERR_WRITEDISK;
      }

      sb.freeBlocksRoot = nextFree; 
      if (writeSuperBlock(mountedFD, &sb) < 0) {
         return ERR_WRITEDISK;
      }
   }

   if(TEST){
      printf("TEST: Completed writing '%d' bytes of data to %s\n",
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
      return ERR_BADFD;
   } else if (temp == NULL) {
      return ERR_NOMOUNTEDDISK;
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
      return ERR_BADFILE;
   }
   mountedFD = openDisk(mountedDisk, 0);
   if ((sb = readSuperBlock(mountedFD)).blockType < 0) {
      return ERR_READDISK;
   }
   if ((in = readInode(mountedFD, sb.rootInodeBlockNum)).blockType < 0) {
      return ERR_READDISK;
   }

   while (strcmp(in.name, temp->filename) != 0) {
      if (in.nextInode == -1) {
         return ERR_BADFILE;
      }
      targetInodeOffset = in.nextInode;
      if ((in = readInode(mountedFD, in.nextInode)).blockType < 0) {
         return ERR_READDISK;
      }
   }

   /*in is the Inode of the file with file descriptor FD*/
   fileExtentOffset = in.startOfFile;
   fb.nextFreeBlock = sb.freeBlocksRoot;
   sb.freeBlocksRoot = targetInodeOffset;
   if (writeFreeBlock(mountedFD, sb.freeBlocksRoot, &fb) < 0) {
      return ERR_WRITEDISK;
   }
   while(fileExtentOffset != -1) {
      if ((fe = readFileExtent(mountedFD, fileExtentOffset)).blockType < 0) {
         return ERR_READDISK;
      }

      fb.nextFreeBlock = sb.freeBlocksRoot;
      sb.freeBlocksRoot = fileExtentOffset;
      if (writeFreeBlock(mountedFD, sb.freeBlocksRoot, &fb) < 0) {
         return ERR_WRITEDISK;
      }
      fileExtentOffset = fe.nextBlock;
   }
   if (writeSuperBlock(mountedFD, &sb) < 0) {
      return ERR_WRITEDISK;
   }

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
      return ERR_BADFD;
   } else if (temp == NULL) {
      return ERR_NOMOUNTEDDISK;
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
      return ERR_BADFILE;
   }
   mountedFD = openDisk(mountedDisk, 0);

   if ((sb = readSuperBlock(mountedFD)).blockType < 0) {
      return ERR_READDISK;
   }
   if((in = readInode(mountedFD, sb.rootInodeBlockNum)).blockType < 0)

   while (strcmp(in.name, temp->filename) != 0) {
      if (in.nextInode == -1) {
         return ERR_BADFILE;
      }
      inodeBlockNum = in.nextInode;
      if ((in = readInode(mountedFD, in.nextInode)).blockType < 0) {
         return ERR_READDISK;
      }
   }

   byteOffset = in.fp;
   
   if (in.startOfFile == -1){
      return -1;
      /*no fileExtent data error*/
   }
   if ((fe = readFileExtent(mountedFD, in.startOfFile)).blockType < 0) {
      return ERR_READDISK;
   }

   if (byteOffset % 253 == 0 && fe.nextBlock == -1){
      return -1;
      /*EOF error*/
   }  
   while(byteOffset - currentByte > 253) {
      if (fe.nextBlock == -1){
         return -1;
         /*pointed OOB error*/
      }
      if ((fe = readFileExtent(mountedFD, fe.nextBlock)).blockType < 0) {
         return ERR_READDISK;
      }

      currentByte+= 253;
     
   }
   /*the byte we want is in fe.data*/
   in.fp++;
   in.lastAccess = time(NULL);
   if (writeInode(mountedFD, inodeBlockNum, &in) < 0) {
      return ERR_WRITEDISK;
   }

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
      return ERR_NOMOUNTEDDISK;
   }

   while(cursor != NULL){
      if(cursor->fd == FD){
         filename = malloc(sizeof(char) * 9);
         strcpy(filename, cursor->filename);
         if(TEST){
            printf("TEST: The file with FD '%d' was found in the Dynamic Resource Table\n", FD);
         }
         break;
      }
      cursor = cursor->next;
   }

   if(filename == NULL){
      if(TEST){
         printf("TEST: The file with FD '%d' was not found in the Dynamic Resource Table\n", FD);
      }
      return ERR_BADFILE;
   }

   for(i = 0; i < numBlocks; i++){
      if ((tempInode = readInode(fd,i)).blockType < 0) {
         return ERR_READDISK;
      }
      if(tempInode.blockType == 2){

         if(!strcmp(filename, tempInode.name)){
            tempInode.fp = offset;
            cur = time(NULL);
            tempInode.lastAccess = cur;
            if (writeInode(fd,i,&tempInode) < 0) {
               return ERR_WRITEDISK;
            }
            break;
         }
      }
   }

   return 0;
}


int setUpFS(int fd, char *fname, int nBlocks) {
   SuperBlock sb;
   FreeBlock everythingElse;
   int i;
   char *initializer;

   sb.blockType = 1;
   sb.magicNum = 0x44;
   sb.rootInodeBlockNum = -1;
   sb.freeBlocksRoot = 1;
   
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

   for (i = 1; i<nBlocks; i++) {
      everythingElse.nextFreeBlock = i+1;
      if (i+1 == nBlocks) {
         everythingElse.nextFreeBlock = -1;
      }
      if (0 != writeBlock(fd, i, &everythingElse)) {
         return -13; /*writing free block failed*/
      }
   }

   if(TEST){
      printf("TEST: The file  system '%s' was opened with fd '%d' and size of '%d' blocks\n", 
         fname, fd, nBlocks);
   }

   return 0; /*success*/
}


/*----------Method stubs for read/write structs----------*/
SuperBlock readSuperBlock(fileDescriptor fd){ 
   SuperBlock sb;
   if(readBlock(fd, 0, &sb) < 0){
      sb.blockType = -1;
   }
   return sb;
}
int writeSuperBlock(fileDescriptor fd, SuperBlock *sb) {
   return writeBlock(fd, 0, sb);
}

Inode readInode(fileDescriptor fd, char blockNum) {
   Inode in;
   if(readBlock(fd, blockNum, &in) < 0){
      in.blockType = -1;
   }
   return in;
}

int writeInode(fileDescriptor fd, char blockNum, Inode *in) {
   return writeBlock(fd, blockNum, in);
}

FileExtent readFileExtent(fileDescriptor fd, char blockNum) {
   FileExtent fe;
   if(readBlock(fd, blockNum, &fe) < 0){
      fe.blockType = -1;
   }
   return fe;
}

int writeFileExtent(fileDescriptor fd, char blockNum, FileExtent *fe) {
   return writeBlock(fd, blockNum, fe);
}

FreeBlock readFreeBlock(fileDescriptor fd, char blockNum) {
   FreeBlock fb;
   if(readBlock(fd, blockNum, &fb) < 0){
      fb.blockType = -1;
   }
   return fb;
}

int writeFreeBlock(fileDescriptor fd, char blockNum, FreeBlock *fb) {
   return writeBlock(fd, blockNum, fb);
}
