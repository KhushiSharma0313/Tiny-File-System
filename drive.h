#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  unsigned char *block[16];
} Drive;

Drive *newDrive();
char *displayDrive(Drive *);
int isUsed(Drive *d, int pos);
unsigned char *dump(Drive *);
int isSpaceAvailable(int bitmap, int max);
int isSpaceAvailableForDir(Drive *d, int rowNum);
bool isPathValid(char **str, char dirOrFile);
void makeDirectory(Drive *d, char **command, char **path);
int getCountOfOnes(int bitmap);
void updateFirstBit(Drive *d, int pos);
int setSecondBit(Drive *d, int numOfOnes, int rowNum);
void setDatabit(Drive *d, char **path, int blockNum, int freeSpaceBitmap,
                int rowNum, char fileOrDir, int mode);
int findActionRow(Drive *d, char **path, int *parentRowNum, int *blockNum,
                  char *fileOrDir, char **command);
int getDataBlock(Drive *d, char nameOfDir, int rowNum);
int getDataRowNum(Drive *d, int dataBlk, int rowNum);
void setLocBit(Drive *d, int blockNum, int freeSpaceBitmap, int rowNum,
               int mode);
void import(Drive *d, char **path);
void ls(Drive *d, char **command, char **path);
void create(char **path);
Drive *openDisk(char **path);
int First0Bit(int i);
int regMatch(char *msgbuf, char fileOrDir, int argPos);
int importFile(Drive *d, char *filePath, char *path);
int freeSecondBit(Drive *d, char **path, int rowNum);
int setSubBlock(Drive *d);
char *trimwhitespace(char *str);