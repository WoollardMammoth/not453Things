#ifndef LIBTINYFS_H
#define LIBTINYFS_H

#include "tinyFS.h"

typedef struct SuperBlock {
   char blockType;
   char magicNum;
   char rootInodeBlockNum;
   char freeBlocksRoot;
} SuperBlock;

typedef struct Inode {
   char blockType;
   char magicNum;
   char name[9];
   unsigned size;
   void *timeStamp;/*change this type*/ 
} Inode;

typedef struct FileExtent {
   char blockType;
   char magicNum;
   char nextBlock;
   char data[BLOCKSIZE-3];
   
} FileExtent;

typedef struct FreeBlock {
   char blockType;
   char magicNum;
   char nextFreeBlock;
} FreeBlock;

typedef struct DRT {
	struct DRT *next;
	fileDescriptor fd;
	char *filename;
	time_t creation;
	time_t lastAccess;
} DRT;

int tfs_mkfs(char *filename, int nBytes);

int tfs_mount(char *diskname);

int tfs_unmount(void);

fileDescriptor tfs_openFile(char *name);

int tfs_closeFile(fileDescriptor FD);

int tfs_writeFile(fileDescriptor FD,char *buffer, int size);

int tfs_deleteFile(fileDescriptor FD);

int tfs_readByte(fileDescriptor FD, char *buffer);

int tfs_seek(fileDescriptor FD, int offset);

int setUpFS(int fd, char *fname, int nBlocks);

#endif
