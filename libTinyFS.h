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
   time_t creationTime;
   time_t lastAccess;
   char empty[256 - 13 - (3*sizeof(time_t))];/*not sure why 3*sizeof(time_t)?*/
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

SuperBlock readSuperBlock(fileDescriptor fd);

int writeSuperBlock(SuperBlock sb);

Inode readInode(char blockNum);

int writeInode(char blockNum, Inode in);

FileExtent readFileExtent(char blockNum);

int writeFileExtent(char blockNum, FileExtent fe);

int writeFreeBlock(char blockNum, FreeBlock fb);

#endif
