#ifndef FETCH_HEADER_FILE
#define FETCH_HEADER_FILE

#include "vaultUtils.h"

#define FETCH_NUMBER_OF_ARGUMENTS 4
#define FETCH_VAULT_FILE_OPEN_FLAG O_RDONLY
#define FETCH_NEW_FILE_OPEN_FLAG O_RDWR | O_CREAT

int fetchFile(int argc, char *argv[]);
int copyFile(int vaultFile, int fetchFile, fileMetadata file);
int copyBlock(int vaultFile, int fetchFile, blockMetadata block);

#endif