#ifndef DEFRAG_HEADER_FILE
#define DEFRAG_HEADER_FILE

#include "vaultUtils.h"

#define DEFRAG_NUMBER_OF_ARGUMENTS 3
#define DEFRAG_VAULT_FILE_OPEN_FLAG O_RDWR 

int defragVault(int argc, char *argv[]);

int moveBlock(int vaultFile, blockMetadata *block, off_t offset);
int moveBlockData(int vaultFile, off_t old, off_t new, size_t size);

#endif