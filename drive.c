#include "drive.h"
#include "displayOptions.h"
#include <ctype.h>
#include <math.h>
#include <regex.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

unsigned char *dump(Drive *d) {
  unsigned char *ret = malloc(256);
  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 16; j++) {
      ret[16 * i + j] = d->block[i][j];
    }
  }
  return ret;
}

// 1111 1111
int isSpaceAvailable(int bitmap, int max) {
  int firstZeroBit = First0Bit(bitmap);
  if (firstZeroBit == max) {
    return -1;
  }
  return firstZeroBit;
}

int First0Bit(int i) {
  i = ~i;
  unsigned int uCount;
  unsigned int u = ((i & (-i)) - 1);
  uCount = u - ((u >> 1) & 033333333333) - ((u >> 2) & 011111111111);
  return ((uCount + (uCount >> 3)) & 030707070707) % 63;
}

int getCountOfOnes(int bitmap) {
  // Initialise count variables
  int count0 = 0, count1 = 0;
  // Iterate through all the bits
  while (bitmap > 0) {
    // If current bit is 1
    if (bitmap & 1) {
      count1++;
    }
    // If current bit is 0
    else {
      count0++;
    }
    bitmap = bitmap >> 1;
  }
  return count1;
}

void ls(Drive *d, char **command, char **path) {
  char **arr = strsplit(path, "/");
  char nameOfDir[10];
  int childLevel = 0;
  for (; arr[childLevel]; ++childLevel) {
    nameOfDir[childLevel] = *arr[childLevel];
  }
  int dataBlock;
  dataBlock = getDataBlock(d, nameOfDir[1], 0);
  int rowNum = getDataRowNum(d, dataBlock, 0);
  for (int i = 2; i <= childLevel - 1; ++i) {
    dataBlock = getDataBlock(d, nameOfDir[i], rowNum);
    rowNum = getDataRowNum(d, dataBlock, rowNum);
  }
  // printf("actionRow: %d\n", rowNum);
  for (int i = 3; i <= 10; i++) {
    if (d->block[rowNum][i] != 0) {
      printf("%c\n", d->block[rowNum][i]);
    }
  }
}

void import(Drive *d, char **path) {
  char **arr = strsplit(path, "/");
  int parentRowNum = 1000;
  int blockNum = -1;
  char fileOrDir = 'F';
  int actionRow =
      findActionRow(d, arr, &parentRowNum, &blockNum, &fileOrDir, "import");
  if (actionRow == -1) {
    printf("Invalid path\n");
    return;
  }
  int tpPos = isSpaceAvailable(d->block[actionRow][2], 8);
  if (tpPos == -1) {
    printf("No space available for File\n");
    return;
  }
  int subBlock = setSubBlock(d);
  int availableRowForData = First0Bit(d->block[0][subBlock]);
  int firstBlockNum = availableRowForData;
  if (subBlock == 1) {
    availableRowForData += 8;
  }
  int freeBlock = setSecondBit(d, tpPos, actionRow);
  setDatabit(d, arr, freeBlock + 2, availableRowForData, actionRow, 'D', 0);
}

int setSubBlock(Drive *d) {
  int subBlock = 0;
  if (d->block[0][0] == 255) {
    if (d->block[0][1] == 255) {
      return -1;
    } else {
      subBlock = 1;
    }
  }
  return subBlock;
}

void makeDirectory(Drive *d, char **command, char **path) {
  char **arr = strsplit(path, "/");
  int subBlock = 0;
  // 1. confirm there is a free block in the root freespace bitmap
  if (d->block[0][0] == 255) {
    if (d->block[0][1] == 255) {
      printf("No space available for new Directory\n");
      return;
    } else {
      subBlock = 1;
    }
  }
  // int freeSpaceBitmap = isSpaceAvailable(d->block[0][subBlock],255);
  int availableRowForData = First0Bit(d->block[0][subBlock]);
  int firstBlockNum = availableRowForData;
  if (subBlock == 1) {
    availableRowForData += 8;
  }
  // printf("Space available\n");
  // 3. use the freespace bitmap to find a block for the new directory
  // 2. confirm the path tp points to a valid location for a new directory
  // 4. use the parent directory bitmap to find an entry for tp from 2nd bit
  int parentRowNum = 1000;
  int blockNum = -1;
  char fileOrDir = 'D';
  int actionRow =
      findActionRow(d, arr, &parentRowNum, &blockNum, &fileOrDir, "");
  if (actionRow == -1) {
    printf("Invalid path\n");
    return;
  }
  int tpPos = isSpaceAvailable(d->block[actionRow][2], 8);
  if (tpPos == -1) {
    printf("No space available for Directory\n");
    return;
  }
  // 5. update the freespace bitmap to remove the selected block
  int freeBlock = setSecondBit(d, tpPos, actionRow);
  updateFirstBit(d, firstBlockNum);
  // Update name, and block pointer: tp
  // 6. update the parent directory bitmap, name, and block pointer: tp
  setDatabit(d, arr, freeBlock + 2, availableRowForData, actionRow, 'D', 0);
  if (parentRowNum > 0 && parentRowNum <= 15) {
    d->block[actionRow][1] = parentRowNum;
  }
  // 7. initialize the bitmap and parent pointer of the new directory block
  // 8. update the parent directory block on the disk
  // 9. update the new directory block on the disk
  // 10. update the freespace bitmap on the disk
  // 11. update the parent directory block in memory
  // 12. update the new directory block in memory
  // 13. update the freespace bitmap in memory
}

int findActionRow(Drive *d, char **path, int *parentRowNum, int *blockNum,
                  char *fileOrDir, char **command) {
  char nameOfDir[10];
  int childLevel = 0;
  for (; path[childLevel]; ++childLevel) {
    nameOfDir[childLevel] = *path[childLevel];
  }
  if (!isupper(nameOfDir[childLevel - 1])) {
    *fileOrDir = 'F';
  } else {
    *fileOrDir = 'D';
  }

  // /A ChildLevel = 2
  // /A/B ChildLevel = 3
  // /A/B/C ChildLevel = 4
  // if (childLevel <= 2) {
  //   return 0;
  // }
  int dataBlock;
  dataBlock = getDataBlock(d, nameOfDir[1], 0);
  if (dataBlock == -1 && childLevel == 2) {
    return 0;
  } else if (dataBlock == -1 && childLevel > 2) {
    return -1;
  }
  int rowNum = getDataRowNum(d, dataBlock, 0);
  // This loop runs well during mkdir because it is not going to /A/B/C, it
  // stops at /A/B Need to change the terminating condition for this loop
  if (strcmp(command, "import") == 0) {
    childLevel++;
  }
  for (int i = 2; i < childLevel - 1; ++i) {
    dataBlock = getDataBlock(d, nameOfDir[i], rowNum);
    *parentRowNum = rowNum;
    *blockNum = dataBlock;
    rowNum = getDataRowNum(d, dataBlock, rowNum);
  }
  return rowNum;
}

int getDataBlock(Drive *d, char nameOfDir, int rowNum) {
  for (int i = 3; i <= 12; i++) {
    if (d->block[rowNum][i] == nameOfDir) {
      return i;
    }
  }
  return -1;
}
int getDataRowNum(Drive *d, int dataBlk, int rowNum) {
  int nameBlock;
  switch (dataBlk) {
  case 3:
    nameBlock = 12;
    // 0000 0000
    break;
  case 4:
    nameBlock = 12;
    break;
  case 5:
    nameBlock = 13;
    break;
  case 6:
    nameBlock = 13;
    break;
  case 7:
    nameBlock = 14;
    break;
  case 8:
    nameBlock = 14;
    break;
  case 9:
    nameBlock = 15;
    break;
  case 10:
    nameBlock = 15;
    break;
  }
  int byte;
  if (dataBlk % 2 == 0) {
    byte = (d->block[rowNum][nameBlock] & 0xF0) >> 4; // upper nibble
  } else {
    byte = d->block[rowNum][nameBlock] & 0x0F; // lower nibble
  }
  return byte;
}
int setSecondBit(Drive *d, int numOfOnes, int rowNum) {
  // Allocate space only if we have not allocated all 8 file/folder spaces
  // already
  unsigned short bitmap = 0;
  if (numOfOnes < 8) {
    bitmap = d->block[rowNum][2];
    bitmap = bitmap | (1 << numOfOnes);
    d->block[rowNum][2] = bitmap;
  } else {
    printf("No space available for storing File/Director under current "
           "directory.\n");
  }
  return getCountOfOnes(bitmap);
}

void setDatabit(Drive *d, char **path, int blockNum, int freeSpaceBitmap,
                int rowNum, char fileOrDir, int mode) {
  char nameOfDir;
  for (int i = 0; path[i]; ++i) {
    // printf(" [%c]", path[i]);
    nameOfDir = *path[i];
  }
  if (blockNum <= 10) {
    // mode == 1 means delete
    // mode == 0 means create
    if (mode == 1) {
      d->block[rowNum][blockNum] = 0;
    } else {
      d->block[rowNum][blockNum] = nameOfDir;
    }
    if (fileOrDir == 'D') {
      setLocBit(d, blockNum, freeSpaceBitmap, rowNum, mode);
      if (mode == 0) {
        d->block[freeSpaceBitmap][1] = rowNum;
      }
    }
  } else {
    printf("No space available for storing File/Director under current "
           "directory.\n");
  }
  return;
}

void setLocBit(Drive *d, int blockNum, int freeSpaceBitmap, int rowNum,
               int mode) {
  int nameBlock;
  switch (blockNum) {
  case 3:
    nameBlock = 12;
    // 0000 0000
    break;
  case 4:
    nameBlock = 12;
    break;
  case 5:
    nameBlock = 13;
    break;
  case 6:
    nameBlock = 13;
    break;
  case 7:
    nameBlock = 14;
    break;
  case 8:
    nameBlock = 14;
    break;
  case 9:
    nameBlock = 15;
    break;
  case 10:
    nameBlock = 15;
    break;
  }
  char byte = d->block[rowNum][nameBlock];
  // Mode == 1 means delete
  if (mode == 1) {
    freeSpaceBitmap = 0;
  }
  if (blockNum % 2 == 0) {
    byte &= 0x0F;                            // Clear out the upper nibble
    byte |= ((freeSpaceBitmap << 4) & 0xF0); // OR in the desired mask
  } else {
    byte &= 0xF0;                     // Clear out the lower nibble
    byte |= (freeSpaceBitmap & 0x0F); // OR in the desired mask
  }
  d->block[rowNum][nameBlock] = byte;
  return;
}
void updateFirstBit(Drive *d, int pos) {
  unsigned short bitmap = 0;
  unsigned short mask = 1;
  unsigned short currZerothPos = d->block[0][0];
  unsigned short currFirstPos = d->block[0][1];
  int activeBit = 0;
  if (currZerothPos == 255 && currFirstPos == 255) {
    return;
  } else if (currZerothPos == 255) {
    activeBit = 1;
  }
  if (pos >= 0 && pos <= 15) {
    // memcpy(&bitmap, d->block[0][activeBit], 2);
    bitmap = d->block[0][activeBit];
    mask = mask << pos;
  } else {
    mask = 0;
  }
  /*
  bitmap = 00000000 00111111
  mask = 00000000 00000001 << 6
  mask = 00000000 01000000
  bitmap | mask = 00000000 01111111
  */
  if (pos >= 0 && pos <= 15) {
    bitmap = bitmap | mask;
    d->block[0][activeBit] = bitmap;
    // memcpy(d->block[0][activeBit], &bitmap, 2);
  }
}

void create(char **path) {
  Drive *nd = newDrive();
  char *driveContent = displayDrive(nd);
  FILE *fptr;
  fptr = fopen(path, "wb");
  if (fptr == NULL) {
    fprintf(stderr, "\nError in creating TFS file\n");
    exit(1);
  }
  int flag = 0;
  flag = fputs(driveContent, fptr);
  if (flag) {
    printf("\nContents of the Drive written "
           "successfully\n");
  } else
    printf("\nError Writing to File!\n");
  // Close the file
  fclose(fptr);
}

Drive *openDisk(char **path) {
  FILE *fptr;
  fptr = fopen(path, "r");
  if (fptr == NULL) {
    fprintf(stderr, "\nError in opening TFS file\n");
    exit(1);
  }
  Drive *nd = newDrive();
  // printf("%s", displayDrive(nd));
  // Skip the first row
  for (int j = 0; j < 16; j++) {
    fscanf(fptr, "%*s"); // %*s ignores the input
  }
  for (size_t i = 0; i < 16; i++) {
    // Skip the first column
    fscanf(fptr, "%*s"); // %*s ignores the input
    for (size_t j = 0; j < 16; j++) {
      int value;
      if (fscanf(fptr, "%x", &value) != 1) {
        fprintf(stderr, "Error reading value at position (%d, %d)\n", i, j);
      }
      nd->block[i][j] = value;
    }
  }
  // Close the file
  fclose(fptr);
  fprintf(stderr, "\nFile Created Successfully\n");
  return nd;
}

Drive *newDrive() {
  Drive *d = malloc(sizeof(Drive));
  for (int i = 0; i < 16; i++) {
    d->block[i] = malloc(16);
  }
  d->block[0][0] = 1; // 0000 0001
  d->block[0][1] = 0; // 0000 0000
  return d;
}
int isUsed(Drive *d, int pos) {
  unsigned short bitmap = 0;
  if (pos >= 0 && pos <= 16)
    memcpy(&bitmap, d->block[0], 2);
  unsigned short mask = 1;
  if (pos >= 0 && pos <= 16)
    mask = mask << pos;
  else
    mask = 0;
  return !((mask & bitmap) == 0);
}
char *displayDrive(Drive *d) {
  const int RETSIZE = 1024;
  char *temp = malloc(RETSIZE);
  char *ret = malloc(RETSIZE);
  ret[0] = '\0';
  strncat(ret, "   ", 1024);
  for (int i = 0; i < 16; i++) {
    sprintf(temp, "  %x", i);
    strncat(ret, temp, RETSIZE);
  }
  strncat(ret, "\n", RETSIZE);

  for (int row = 0; row < 16; row++) {
    sprintf(temp, "%x: ", row);
    strncat(ret, temp, RETSIZE);
    for (int col = 0; col < 16; col++) {
      sprintf(temp, "%3.2x", d->block[row][col]);
      strncat(ret, temp, RETSIZE);
    }
    strncat(ret, "\n", RETSIZE);
  }
  return ret;
}

int regMatch(char *msgbuf, char fileOrDir, int argPos) {
  regex_t regex;
  int reti, actRetVal = -1;
  /* check tp - legal TFS file name */
  if (argPos == 0) {
    if (fileOrDir == 'D') {
      reti = regcomp(&regex, "^(/)([A-Z]|/)*([A-Z])$", REG_EXTENDED);
    } else {
      reti = regcomp(&regex, "^(/)([A-Za-z]|/)*([A-Za-z])$", REG_EXTENDED);
    }
  }
  if (reti) {
    fprintf(stderr, "Could not compile regex\n");
    exit(1);
  }

  /* Execute regular expression */
  reti = regexec(&regex, msgbuf, 0, NULL, 0);
  if (!reti) {
    actRetVal = 0;
  } else if (reti == REG_NOMATCH) {
    actRetVal = -1;
  } else {
    regerror(reti, &regex, msgbuf, sizeof(msgbuf));
    fprintf(stderr, "Regex match failed: %s\n", msgbuf);
  }

  /* Free memory allocated to the pattern buffer by regcomp() */
  regfree(&regex);
  return actRetVal;
}

char *trimwhitespace(char *str) {
  char *end;
  // Trim leading space
  while (isspace((unsigned char)*str))
    str++;
  if (*str == 0) // All spaces?
    return str;
  // Trim trailing space
  end = str + strlen(str) - 1;
  while (end > str && isspace((unsigned char)*end))
    end--;
  // Write new null terminator character
  end[1] = '\0';
  return str;
}

int importFile(Drive *d, char *filePath, char *path) {
  struct stat st;
  int subBlock = 0;
  if (d->block[0][0] == 255) {
    if (d->block[0][1] == 255) {
      printf("No space available for importing file\n");
      return -1;
    } else {
      subBlock = 1;
    }
  }
  // Allocating space for file under parent directory
  import(d, path);
  int availableRowForData = First0Bit(d->block[0][subBlock]);
  int availableRowForDataBaseEight = availableRowForData;
  if (subBlock == 1) {
    availableRowForData += 8;
  }
  // Check if the file exists
  if (stat(filePath, &st) == -1) {
    perror("stat");
    exit(EXIT_FAILURE);
  }
  int size = st.st_size;
  double sizeDbl = size;
  int numOfBlocksNeeded;
  if (size < 16) {
    numOfBlocksNeeded = 0;
  } else {
    numOfBlocksNeeded = ceil(sizeDbl / 16);
  }
  numOfBlocksNeeded++;
  int numOfBlocksAvailable = 16 - availableRowForData;
  if (numOfBlocksNeeded <= numOfBlocksAvailable) {
    FILE *fptr;
    fptr = fopen(filePath, "r");
    if (fptr == NULL) {
      fprintf(stderr, "\nError in opening TFS file\n");
      exit(1);
    }
    // Read the contents of the file
    char buffer[size];
    char c;
    for (int i = 0; (c = fgetc(fptr)) != EOF; i++) {
      buffer[i] = c;
    }
    fclose(fptr);
    // If filesize is less than 15 bytes, write in index block
    d->block[availableRowForData][0] = size;
    updateFirstBit(d, availableRowForDataBaseEight);
    if (numOfBlocksNeeded == 1) {
      for (int i = 1, j = 0; i < size; i++, j++) {
        d->block[availableRowForData][i] = buffer[j];
      }
    } else {
      // Make index block, get free row numbers, allocate these rows for file
      // contents
      int fileBlockNums[numOfBlocksNeeded - 1];
      for (int fileBlockCtr = 1; fileBlockCtr < numOfBlocksNeeded;
           fileBlockCtr++) {
        int nextAvailableBlockForData =
            isSpaceAvailable(d->block[0][subBlock], 8);
        if (nextAvailableBlockForData == -1 && subBlock == 1) {
          printf("No space available for File\n");
          return -1;
        } else if (nextAvailableBlockForData == -1 && subBlock == 0) {
          subBlock = 1;
          nextAvailableBlockForData =
              isSpaceAvailable(d->block[0][subBlock], 8);
        }
        int offset = 0;
        if (subBlock == 1) {
          offset = 8;
        }
        fileBlockNums[fileBlockCtr - 1] = nextAvailableBlockForData + offset;
        d->block[availableRowForData][fileBlockCtr] =
            nextAvailableBlockForData + offset;
        updateFirstBit(d, nextAvailableBlockForData);
      }
      // Write file contents
      int datactr = 0;
      for (int rowCtr = 0; rowCtr < numOfBlocksNeeded - 1; rowCtr++) {
        // Write the file contents to blocks
        // d->block[availableRowForData][rowCtr+1] = fileBlockNums[rowCtr];
        for (int colCtr = 0; colCtr < 16 && datactr < size;
             colCtr++, datactr++) {
          d->block[fileBlockNums[rowCtr]][colCtr] = buffer[datactr];
        }
      }
    }
  } else {
    printf("Not enough space to import file\n");
    return -1;
  }
  return 0;
}

int removeFileOrDir(Drive *d, char **path) {
  char **arr = strsplit(path, "/");
  int parentRowNum = 1000;
  int blockNum = -1;
  char fileOrDir = 'F';
  int actionRow =
      findActionRow(d, arr, &parentRowNum, &blockNum, &fileOrDir, "");
  if (actionRow == -1) {
    printf("Invalid path\n");
    return -1;
  }
  int freedBlockNum = freeSecondBit(d, arr, actionRow);
  setDatabit(d, arr, freedBlockNum, 10000, actionRow, fileOrDir, 1);
  return 0;
}

int freeSecondBit(Drive *d, char **path, int rowNum) {
  char nameOfDir[10];
  int childLevel = 0;
  for (; path[childLevel]; ++childLevel) {
    nameOfDir[childLevel] = *path[childLevel];
  }
  int blockNum = -1;
  for (int i = 3; i <= 10; i++) {
    if (d->block[rowNum][i] == nameOfDir[childLevel - 1]) {
      blockNum = i;
      break;
    }
  }
  // Release space
  unsigned int number = d->block[rowNum][2]; // Binary: 0000 0001
  int bit_position =
      blockNum - 3; // The position of the bit to set (zero-based indexing)
  // Use a bitmask with a 0 in the desired bit position and AND it with the
  // number.
  number &= ~(1 << bit_position);
  d->block[rowNum][2] = number;
  printf("Drive before rm:\n");
  return blockNum;
}