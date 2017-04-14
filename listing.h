#ifndef LISTING_HEADER_FILE
#define LISTING_HEADER_FILE

#include "vaultUtils.h"

#define LISTING_NUMBER_OF_ARGUMENTS 3
#define LISTING_FILE_OPEN_FLAG O_RDONLY 

int listing(int argc, char *argv[]);
void printVaultFileMetadata(repositoryMetadata repo, char* vaultFilePath);
void printFilesMetaData(fileMetadata *files, int numberOfFiles);
void printsize(ssize_t size, char *str);

#endif