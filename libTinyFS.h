#ifndef LIBTINYFS_H
#define LIBTINYFS_H

#include "tinyFS.h"

typedef struct SuperBlock {
   char blockType;
   char magicNum;
   char rootInodeBlockNum;
   char freeBlocksRoot;
   char empty[BLOCKSIZE - 4];
} SuperBlock;

typedef struct Inode {
   char blockType;
   char magicNum;
   char name[9];
   char startOfFile;
   char nextInode;
   char fp;
   time_t creationTime;
   time_t lastAccess;
   char empty[256 - 14 - (3*sizeof(time_t))];/*not sure why 3*sizeof(time_t)? [Should be 2]*/
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
   char empty[BLOCKSIZE - 3];
} FreeBlock;

typedef struct DRT {
	struct DRT *next;
	fileDescriptor fd;
	char *filename;
	time_t creation;
	time_t lastAccess;
} DRT;

//Standard library functions
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

//Helper functions
SuperBlock readSuperBlock();

SuperBlock readSuperBlock(fileDescriptor fd);

int writeSuperBlock(fileDescriptor fd, SuperBlock *sb);

Inode readInode(fileDescriptor fd, char blockNum);

int writeInode(fileDescriptor fd, char blockNum, Inode *in);

FileExtent readFileExtent(fileDescriptor fd, char blockNum);

int writeFileExtent(fileDescriptor fd, char blockNum, FileExtent *fe);

FreeBlock readFreeBlock(fileDescriptor fd, char blockNum);

int writeFreeBlock(fileDescriptor fd, char blockNum, FreeBlock *fb);

FreeBlock readFreeBlock(fileDescriptor fd, char blockNum);

#endif
