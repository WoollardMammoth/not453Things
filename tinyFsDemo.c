#include <stdio.h>
#include "tinyFS.h"
#include "libDisk.h"
#include "libTinyFS.h"

#define NUM_TESTS 5

int main() {
   int curTest;
   char diskName[] = "diskX.dsk";

 
   for (curTest = 0; curTest < NUM_TESTS; curTest++) {
      diskName[4] = '0' + curTest;

      if (tfs_mkfs(diskName, BLOCKSIZE * 5) < 0) {
         printf("Failed to create file system %s\n", diskName);
         //Display errno
      }
      else {   
         printf("Successfully created file system %s.\n", diskName);

         /* Test empty file system */ 
         if (curTest == 0) {
            if (tfs_mount(diskName) < 0) {
               printf("Failed to mount %s\n", diskName);
               //Display errno
            }
            else {
               printf("Successfully mounted %s\n", diskName);
            }
               

         if (curTest == 1) {
         }

         if (curTest == 2) {
         }

         if (curTest == 3) {
         }

         if (curTest == 4) {
         }


         }
      }
   }
   
   
   return 0;
}
