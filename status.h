#ifndef STATUS_HEADER_FILE
#define STATUS_HEADER_FILE

#include "vaultUtils.h"

#define STATUS_NUMBER_OF_ARGUMENTS 3
#define STATUS_VAULT_FILE_OPEN_FLAG O_RDONLY 

int getStatus(int argc, char *argv[]);
void printStatus(int numberOfFiles, size_t totalSize, double fragRatio);
size_t getTotalSize(fileMetadata *files);
double getFragmentationRatio(fileMetadata *files);
size_t getGapsLength(blockMetadata **blocks, int numberOfBlocks);

#endif