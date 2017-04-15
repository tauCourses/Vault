#ifndef INIT_HEADER_FILE
#define INIT_HEADER_FILE

#include "vaultUtils.h"

#define INIT_NUMBER_OF_ARGUMENTS 4
#define INIT_FILE_OPEN_FLAG O_RDWR | O_CREAT | O_TRUNC

int initVault(int argc, char *argv[]);
int parseDataAmount(char* arg, size_t *fileSize);
int setDefaultValues(repositoryMetadata *repo, fileMetadata *files, size_t size);
int setFileSize(int file, size_t size);

#endif