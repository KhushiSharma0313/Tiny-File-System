#include "drive.h"
#include "strsplit.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
  Drive *d = newDrive();
  printf("%s", displayDrive(d));
  for (int i = 0; i < 16; i++) {
    printf("block %d is used: %d\n", i, isUsed(d, i));
  }
  char str[1000]; //= "import ./alpha.txt /A/a";
  printf("Enter command: ");
  // fgets(str, 1000, stdin);
  scanf("%[^\n]%*c", str);
  while (strcmp(str, "exit") != 0) {
    //***************************************
    memcpy(str, trimwhitespace(str), 1000);
    //***************************************
    char **arr = strsplit(str, " ");
    if (arr) {
      char *command = arr[0];
      char *cmdOpt1 = arr[1];
      char *cmdOpt2 = arr[2];
      if (strcmp(command, "import") == 0) {
        printf("import\n");
        if (strlen(cmdOpt1) == 0 || regMatch(cmdOpt2, 'F', 0) != 0) {
          printf("Invalid Path\n");
        } else {
          importFile(d, cmdOpt1, cmdOpt2);
        }
      } else if (strcmp(command, "ls") == 0) {
        printf("ls\n");
        if (strlen(cmdOpt1) == 0) {
          cmdOpt1 == "/";
        }
        ls(d, command, cmdOpt1);
      } else if (strcmp(command, "display") == 0) {
        printf("display\n");
      } else if (strcmp(command, "open") == 0) {
        printf("open\n");
        Drive *nd = openDisk(cmdOpt1);
        memcpy(d, nd, sizeof(Drive));
      } else if (strcmp(command, "create") == 0) {
        printf("create\n");
        create(cmdOpt1);
      } else if (strcmp(command, "mkdir") == 0) {
        printf("mkdir\n");
        if (regMatch(cmdOpt1, 'D', 0) != 0) {
          printf("%s Invalid Path\n", cmdOpt1);
        } else {
          makeDirectory(d, command, cmdOpt1);
        }
      } else if (strcmp(command, "rm") == 0) {
        printf("rm\n");
        printf("Drive before rm:\n");
        printf("%s", displayDrive(d));
        removeFileOrDir(d, cmdOpt1);
      } else {
        printf("%s is Invalid command\n", command);
      }
      free(arr);
    }
    printf("%s", displayDrive(d));
    printf("Enter command: ");
    fgets(str, 1000, stdin);
  }
}