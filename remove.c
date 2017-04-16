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

#include "remove.h"

int removeFile(int argc, char *argv[])
{
	repositoryMetadata repo;
	fileMetadata files[MAX_NUMBER_OF_FILES];
	int vaultFile, removeFileNumber, lastFile;

	struct timeval time;

	if(argc != REMOVE_NUMBER_OF_ARGUMENTS)
	{
		printf("Wrong number of arguments\n");
		return -1;
	}

	if(openFile(&vaultFile, argv[1], REMOVE_VAULT_FILE_OPEN_FLAG,0) == -1)
		return -1;

	if(getRepositoryMetadata(vaultFile, &repo) == -1)
		return -1;

	if(getFilesMetadata(vaultFile, files) == -1)
		return -1;

	if(gettimeofday(&time, NULL) != 0)
	{
		printf("Failed to get current time: %s\n", strerror( errno ) );
		return -1;
	}

	removeFileNumber = checkIfFileExist(argv[3], files);
	if(removeFileNumber == -1)
	{
		printf("File not exist\n");
		return -1;
	}
	
	if(removeDelimitersInFileBlocks(vaultFile, files[removeFileNumber].blocks) == -1)
		return -1;
	
	lastFile = switchWithLastFile(vaultFile, files, removeFileNumber);
	if(lastFile == -1)
		return -1;

	if(removeLastFile(vaultFile, lastFile) == -1)
		return -1;

	repo.files--;
	repo.modificationTime = time.tv_sec;
	
	if(saveRepositoryMetadata(vaultFile, repo) == -1)
		return -1;

	if(close(vaultFile) < 0)
	{
		printf("ERROR: unable to close file %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

int removeDelimitersInFileBlocks(int vaultFile, blockMetadata *blocks)
{
	for(int i=0; i<NUMBER_OF_BLOCKS_PER_FILE; i++)
	{
		if(blocks[i].size == 0)
			break;
		if(writeCharToFile(vaultFile, blocks[i].offset, '\0', DELIMETER_SIZE) == -1)
			return -1;
		if(writeCharToFile(vaultFile, blocks[i].offset + blocks[i].size - DELIMETER_SIZE, '\0', DELIMETER_SIZE) == -1)
			return -1;
	}
	return 0;
}

int switchWithLastFile(int vaultFile, fileMetadata *files, int removeFileNumber)
{
	int lastFile = removeFileNumber;
	for(;lastFile < MAX_NUMBER_OF_FILES && files[lastFile].creationTime != 0; lastFile++);
	
	lastFile--; 

	if(lastFile == removeFileNumber)
		return removeFileNumber;

	if(saveFileLine(vaultFile, files[lastFile] ,removeFileNumber) == -1)
		return -1;

	return lastFile;
}

int removeLastFile(int vaultFile, int lastFile)
{
	fileMetadata zeroFile;
	strcpy(zeroFile.name, "" );
	zeroFile.size = 0;
	zeroFile.permissions = 0;
	zeroFile.creationTime = 0;
	
	for(int i=0;i<NUMBER_OF_BLOCKS_PER_FILE;i++)
	{
		zeroFile.blocks[i].size = 0;
		zeroFile.blocks[i].offset = 0;
	}

	if(saveFileLine(vaultFile, zeroFile ,lastFile) == -1)
		return -1;
	return 0;
}