#ifndef INSERT_HEADER_FILE
#define INSERT_HEADER_FILE

#include "vaultUtils.h"

#define INSERT_NUMBER_OF_ARGUMENTS 4
#define INSERT_VAULT_FILE_OPEN_FLAG O_RDWR 
#define INSERT_NEW_OPEN_FLAG O_RDONLY

int insertFile(int argc, char *argv[]);
int findBlocksToSaveFile(ssize_t fileSize, blockMetadata *availableBlocks, int numberOfBlocks, blockMetadata *unUsed, blockMetadata **choosenBlocks);

int createNewFile(fileMetadata *newFile, fileMetadata *files, char * newFileName, off_t st_size, mode_t st_mode, time_t time, blockMetadata *blocks, int numberOfBlocks);
int copyFileToBlocks(int vaultFile, int insertFile, size_t fileSize, blockMetadata *blocks, int numberOfBlocks);
off_t copyBlockToVault(int vaultFile, int insertFile, off_t vaultFileOffset, off_t insertFileOffset, size_t size);

#endif