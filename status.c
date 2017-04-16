#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h> 
#include <libgen.h>

#include "status.h"

int getStatus(int argc, char *argv[])
{
	repositoryMetadata repo;
	fileMetadata files[MAX_NUMBER_OF_FILES];
	int vaultFile;
	
	if(argc != STATUS_NUMBER_OF_ARGUMENTS)
	{
		printf("Wrong number of arguments\n");
		return -1;
	}

	if(openFile(&vaultFile, argv[1], STATUS_VAULT_FILE_OPEN_FLAG,0) == -1)
		return -1;

	if(getRepositoryMetadata(vaultFile, &repo) == -1)
		return -1;

	if(getFilesMetadata(vaultFile, files) == -1)
		return -1;
	
	size_t totalSize = getTotalSize(files);
	double fragRatio = getFragmentationRatio(files);
	if(fragRatio == -1)
		return -1;

	printStatus(repo.files, totalSize, fragRatio);

	if(close(vaultFile) < 0)
	{
		printf("ERROR: unable to close file %s\n", strerror(errno));
		return -1;
	}

	return 0;
}
void printStatus(int numberOfFiles, size_t totalSize, double fragRatio)
{
	printf("Number of files:\t%d\n", numberOfFiles);
	printf("Total size:\t\t%zuB\n", totalSize);
	printf("Fragmentation ratio:\t%.2f\n", fragRatio);
}
size_t getTotalSize(fileMetadata *files)
{
	size_t totalSize = 0;
	for(int i=0;i<MAX_NUMBER_OF_FILES;i++)
		totalSize += files[i].size;

	return totalSize;
}
double getFragmentationRatio(fileMetadata *files)
{
	blockMetadata **blocks = NULL;
	int numberOfBlocks = getAllDataBlocks(files, &blocks);
	if(numberOfBlocks == -1)
		return -1;
	else if(numberOfBlocks == 0)
		return 0.0;
	
	size_t consumedSize = blocks[numberOfBlocks-1]->offset + blocks[numberOfBlocks-1]->size - blocks[0]->offset;
	size_t gaps = getGapsLength(blocks,numberOfBlocks);
	
	free(blocks);
	
	if(consumedSize == 0)
		return 0.0;

	double ratio = gaps/(double)consumedSize;
	return ratio;
}
size_t getGapsLength(blockMetadata **blocks, int numberOfBlocks)
{
	size_t gaps = 0;
	if(numberOfBlocks <= 1)
		return gaps;

	for(int i=1;i<numberOfBlocks;i++)
		gaps += blocks[i]->offset - (blocks[i-1]->offset + blocks[i-1]->size);
	
	return gaps;
}