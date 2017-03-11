#ifndef LIBTINYFS_H
#define LIBTINYFS_H

typedef struct superBlock {
   char blockType = 1;
   char magicNum = 0x44;
   char rootInodeBlockNum;
   void *freeBlocks;
} superBlock;

typedef struct inode {
   char blockType = 2;
   char magicNum = 0x44;
   char name[9];
   unsigned size;
   void *timeStamp;/*change this type*/ 
} inode;

typedef struct fileExtent {
   char blockType = 3;
   char magicNum = 0x44;
   char nextBlock = 0x00;
   char data[BLOCKSIZE-3] = {0x00};
   
}

typedef struct freeBlock {
   char blockType = 4;
   char magicNum = 0x44;
   char nextFreeBlock;
}


int tfs_mkfs(char *filename, int nBytes);

int tfs_mount(char *diskname);

int tfs_unmount(void);

fileDescriptor tfs_openFile(char *name);

int tfs_closeFile(fileDescriptor FD);

int tfs_writeFile(fileDescriptor FD,char *buffer, int size);

int tfs_deleteFile(fileDescriptor FD);

int tfs_readByte(fileDescriptor FD, char *buffer);

int tfs_seek(fileDescriptor FD, int offset);

#endif
