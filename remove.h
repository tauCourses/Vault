#ifndef REMOVE_HEADER_FILE
#define REMOVE_HEADER_FILE

#include "vaultUtils.h"

#define INSERT_NUMBER_OF_ARGUMENTS 4
#define REMOVE_VAULT_FILE_OPEN_FLAG O_RDWR 

int removeFile(int argc, char *argv[]);
int removeDelimitersInFileBlocks(int vaultFile, blockMetadata *blocks);
int switchWithLastFile(int vaultFile, fileMetadata *files, int removeFileNumber);
int removeLastFile(int vaultFile, int lastFile);

#endif