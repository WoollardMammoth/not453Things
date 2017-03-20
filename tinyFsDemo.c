#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tinyFS.h"
#include "libDisk.h"
#include "libTinyFS.h"
#include "TinyFS_errno.h"

#define NUM_TESTS 3

#define BLOCKSIZE 256
#define NUM_TEST_DISKS 4 /* number of disks to test with */


#define NUM_BLOCKS 50 /* total number of blocks on each disk */
#define NUM_TEST_BLOCKS 10
#define TEST_BLOCKS {25,39,8,9,15,21,25,33,35,42}


int main() {
   int curTest;
   char diskName[] = "diskX.dsk"; 
   int err;
   int fd, tempFd, closeFd;
   int fda, fdb;
   printf(">>> DEMO FOR TinyFS <<<");
 
   for (curTest = 0; curTest < NUM_TESTS; curTest++) {
      diskName[4] = '0' + curTest;


      printf("\n\n*************************************\n");


   	 if ((err = tfs_mkfs(diskName, BLOCKSIZE * DEFAULT_DISK_SIZE)) < 0) {
         printf("> DEMO: Failed to create file system %s with error code '%d'\n", diskName, err);
         //Display errno
      }
      else {   
         printf("\n> DEMO %d: Successfully created file system %s.\n", curTest + 1, diskName);

         if (curTest == 0) {

         	printf("\nDEMO TEST 1: MOUNT AND UNMOUNT\n\n");
         	printf("-> Functionality\n");
         	printf("-> 1. Attempt to mount a file (should not have currently mounted file)\n");
         	printf("-> 2. Attempt to unmount a file\n");
         	printf("-> 3. Attempt to unmount a file when there is no file mounted\n");
         	printf("-> 4. Attempt to mount a file with no file mounted\n");
         	printf("-> 5. Attempt to mount a file that is already mounted\n\n");

         	

         	if ((err = tfs_mount(diskName)) < 0) {
               printf("> DEMO 1: Failed to mount %s with error code '%d'\n", diskName, err);
               //Display errno
            }
            else {
               printf("> DEMO 1: Successfully mounted %s\n", diskName);
            }
         	if((err = tfs_unmount()) < 0){
         		printf("> DEMO 2: Failed to unmount %s with error code '%d'\n", diskName, err);
         	}
         	else {
               printf("> DEMO 2: Successfully unmounted %s\n", diskName);
            }

            if((err = tfs_unmount()) < 0){
         		printf("> DEMO 3: Failed to unmount %s with error code '%d'\n", diskName, err);
         	}
         	else {
               printf("> DEMO 3: Successfully unmounted %s\n", diskName);
            }

            if ((err = tfs_mount(diskName)) < 0) {
               printf("> DEMO 4: Failed to mount %s with error code '%d'\n", diskName, err);
               //Display errno
            }
            else {
               printf("> DEMO 4: Successfully mounted %s\n", diskName);
            }

            if ((err = tfs_mount(diskName)) < 0) {
               printf("> DEMO 5: Failed to mount %s with error code '%d'\n", diskName, err);
               //Display errno
            }
            else {
               printf("> DEMO 5: Successfully mounted %s\n", diskName);
            }

        } 


        /* Test open file and close file */
	    if (curTest == 1) {

	    	printf("\n\nDEMO TEST 2: OPEN AND CLOSE FILE\n\n");
         printf("-> Functionality\n");
        	printf("-> 1. Attempt to close a file that is not open\n");
        	printf("-> 2. Attempt to open a file that is not open\n");
         printf("-> 3. Attempt to open a file that is open\n");
         printf("-> 4. Attempt to close a file that is open\n");
         printf("-> 5. Attempt to open multiple files\n\n");
         tfs_mount(diskName);

	    	if ((fd = tfs_closeFile(100)) < 0){
	    		printf("> DEMO 1: Unable to close the file with FD '100' with error code '%d'\n", fd);
	    	}
	    	else{
	    		printf("> DEMO 1: Successfully closed file with FD '100'\n");
	    	}

         fd = tfs_openFile("newfile");
	    	if (fd == ERR_NOSPACE) {
	    		printf("> DEMO 2: Failed to open 'newFile' with error code '%d'\n", fd);
	    	}
         else if (fd == NO_ACTION_FILE_ALREADY_OPEN) {
               printf("> DEMO 2: File already opened\n");
         }
	    	else{
	    		printf("> DEMO 2: File 'newFile' was opened with FD '%d'\n", fd);
            closeFd = fd;
	    	}
       

         fd = tfs_openFile("newfile");
	    	if (fd == ERR_NOSPACE) {
	    		printf("> DEMO 3: Failed to open 'newFile' with error code '%d'\n", fd);
	    	}
         else if (fd == NO_ACTION_FILE_ALREADY_OPEN) {
               printf("> DEMO 3: File already opened\n");
         }
	    	else{
	    		printf("> DEMO 3: File 'newFile' was opened with FD '%d'\n", fd);
	    	}
         
	    	if((tempFd = tfs_closeFile(closeFd)) < 0){
	    		printf("> DEMO 4: Unable to close the file %d with error code %d\n", fd, tempFd);
	    	}
	    	else{
	    		printf("> DEMO 4: Successfully closed file with FD '%d'\n", closeFd);
	    	}

         if((fda = tfs_openFile("fileA")) >= 0 && (fdb = tfs_openFile("fileB")) >= 0) {
            printf("> DEMO 5: Sucessfully opened multiple files with file descriptors %d and %d\n", fda, fdb);
         }
         else {
            printf("> DEMO 5: Failed opening multiple files with error codes %d and %d\n", fda, fdb);
         }
	    }


	    if (curTest == 2) {
          printf("\n\nDEMO TEST 3: WRITE AND READ \n\n");
          printf("Functionality\n");
          printf("-> 1. Attempt to write 'Goodbye World, finals have come' to file\n");

          tfs_mount(diskName);
          
          fd = tfs_openFile("writefile");

          if ((err = tfs_writeFile(fd, "Goodbye World, finals have come", 32)) < 0) {
             printf("Write to file failed with %d\n", err);
          } 


       }  
               

	    if (curTest == 3) {
	    }

	    if (curTest == 4) {
	    }
      }
   }
   
   
   return 0;
}
