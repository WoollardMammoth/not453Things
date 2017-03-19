#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tinyFS.h"
#include "libDisk.h"
#include "libTinyFS.h"

#define NUM_TESTS 2

#define BLOCKSIZE 256
#define NUM_TEST_DISKS 4 /* number of disks to test with */


#define NUM_BLOCKS 50 /* total number of blocks on each disk */
#define NUM_TEST_BLOCKS 10
#define TEST_BLOCKS {25,39,8,9,15,21,25,33,35,42}


int main() {
   int curTest;
   char diskName[] = "disk0.dsk";
   int err;
   int fd, tempFd;

   printf("\n\n>>> DEMO FOR TinyFS <<<\n\n");
 
   for (curTest = 0; curTest < NUM_TESTS; curTest++) {
      //diskName[4] = '0' + curTest;

   	 if ((err = tfs_mkfs(diskName, BLOCKSIZE * DEFAULT_DISK_SIZE)) < 0) {
         printf("> DEMO: Failed to create file system %s with error code '%d'\n", diskName, err);
         //Display errno
      }
      else {   
         printf("> DEMO: Successfully created file system %s.\n", diskName);

         if (curTest == 0) {

         	printf("\n\nDEMO: TEST 1\n\n");
         	printf("-> Functionality\n");
         	printf("-> 1. Attempt to mount a file\n");
         	printf("-> 2. Attempt to unmount a file\n");
         	printf("-> 3. Attempt to unmount a file when there is no file mounted\n");
         	printf("-> 4. Attempt to mount a file with no file mounted\n");
         	printf("-> 5. Attempt to mount a file that is already mounted\n\n");

         	

         	if ((err = tfs_mount(diskName)) < 0) {
               printf("> DEMO: Failed to mount %s with error code '%d'\n", diskName, err);
               //Display errno
            }
            else {
               printf("> DEMO: Successfully mounted %s\n", diskName);
            }
         	if((err = tfs_unmount()) < 0){
         		printf("> DEMO: Failed to unmount %s with error code '%d'\n", diskName, err);
         	}
         	else {
               printf("> DEMO: Successfully unmounted %s\n", diskName);
            }

            if((err = tfs_unmount()) < 0){
         		printf("> DEMO: Failed to unmount %s with error code '%d'\n", diskName, err);
         	}
         	else {
               printf("> DEMO: Successfully unmounted %s\n", diskName);
            }

            if ((err = tfs_mount(diskName)) < 0) {
               printf("> DEMO: Failed to mount %s with error code '%d'\n", diskName, err);
               //Display errno
            }
            else {
               printf("> DEMO: Successfully mounted %s\n", diskName);
            }

            if ((err = tfs_mount(diskName)) < 0) {
               printf("> DEMO: Failed to mount %s with error code '%d'\n", diskName, err);
               //Display errno
            }
            else {
               printf("> DEMO: Successfully mounted %s\n", diskName);
            }

        } 
        /* Test open file and close file */
	    if (curTest == 1) {

	    	printf("\n\nDEMO: TEST 2\n\n");
         	printf("-> Functionality\n");
         	printf("-> 1. Attempt to close a file that is not open\n");
         	printf("-> 2. Attempt to open a file a file\n");
         	printf("-> 3. Attempt to close a file that is open\n\n");

	    	if((fd = tfs_closeFile(100)) < 0){
	    		printf("> DEMO: Unable to close the file with FD '100' with error code %d\n", fd);
	    	}
	    	else{
	    		printf("> DEMO Successfully closed file with FD '100'\n");
	    	}

	    	if((fd = tfs_openFile("newFile")) < 0){
	    		printf("> DEMO: Failed to open 'newFile' with error code '%d'\n", fd);
	    	}
	    	else{
	    		printf("> DEMO: File 'newFile' was opened with FD '%d'\n", fd);
	    	}

	    	if((tempFd = tfs_closeFile(fd)) < 0){
	    		printf("> DEMO: Unable to close the file with error code %d\n", tempFd);
	    	}
	    	else{
	    		printf("> DEMO: Successfully closed file with FD '%d'\n", fd);
	    	}
	    }

	    if (curTest == 2) {
	    }

	    if (curTest == 3) {
	    }

	    if (curTest == 4) {
	    }
      }
   }
   
   
   return 0;
}
