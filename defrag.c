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

#include "defrag.h"

int defragVault(int argc, char *argv[])
{
	repositoryMetadata repo;
	fileMetadata files[MAX_NUMBER_OF_FILES];
	
	int vaultFile;
	
	struct timeval time;

	blockMetadata **blocks = NULL;
	int numberOfBlocks; 

	if(argc != DEFRAG_NUMBER_OF_ARGUMENTS)
	{
		printf("Wrong number of arguments\n");
		return -1;
	}

	if(openFile(&vaultFile, argv[1], DEFRAG_VAULT_FILE_OPEN_FLAG,0) == -1)
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
	
	numberOfBlocks = getAllDataBlocks(files, &blocks);
	if(numberOfBlocks == -1)
		return -1;
	else if(numberOfBlocks == 0)
		return 0;

	if(blocks[0]->offset > BLOCKS_OFFSET)
		moveBlock(vaultFile, blocks[0], BLOCKS_OFFSET);

	for(int i=1; i<numberOfBlocks; i++)
	{
		if(blocks[i]->offset > blocks[i-1]->offset + blocks[i-1]->size)
			if(moveBlock(vaultFile, blocks[i], blocks[i-1]->offset + blocks[i-1]->size) == -1)
				return -1;
	}

	//update files(with updated blocks):
	if(saveFilesMetadata(vaultFile,files) == -1)
		return -1;
	//update repo modification time
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

int moveBlock(int vaultFile, blockMetadata *block, off_t offset)
{
	//remove delimiters
	if(writeCharToFile(vaultFile, block->offset, '\0', DELIMETER_SIZE) == -1)
		return -1;
	if(writeCharToFile(vaultFile, block->offset + block->size - DELIMETER_SIZE, '\0', DELIMETER_SIZE) == -1)
		return -1;
	//move block
	if(moveBlockData(vaultFile, block->offset, offset, block->size) == -1)
		return -1;
	//print delimiters
	if(writeCharToFile(vaultFile, offset, START_BLOCK_DELIMITER, DELIMETER_SIZE) == -1)
		return -1;
	if(writeCharToFile(vaultFile, offset + block->size - DELIMETER_SIZE, END_BLOCK_DELIMITER, DELIMETER_SIZE) == -1)
		return -1;
	//update block
	block->offset = offset;
	return 0;
}
int moveBlockData(int vaultFile, off_t old, off_t new, size_t size)
{
	char buffer[FILE_BUFFER_SIZE];
	size_t readingSize = size;
	off_t currentPosition = 0;
	ssize_t len;

	while(readingSize > 0) //while more bytes need to be processed
	{
		if(lseek(vaultFile, old+currentPosition, SEEK_SET) < 0) 
		{
			printf("Error seek in vault file: %s\n", strerror(errno));
			return -1;
		}	
		
		if(readingSize<FILE_BUFFER_SIZE) //read the minimum between readingSize and bufferSize
			len = readAll(vaultFile, buffer, readingSize);
		else
			len = readAll(vaultFile, buffer, FILE_BUFFER_SIZE);

		if(len < 0) //check that the read call succeeded 
		{
			printf("Error reading from input file: %s\n", strerror(errno));
			return -1;
		}

		if(lseek(vaultFile, new+currentPosition, SEEK_SET) < 0) 
		{
			printf("Error seek in vault file: %s\n", strerror(errno));
			return -1;
		}	

		if(writeAll(vaultFile, buffer, len)<0)
			return -1;
			

		readingSize -= len;
		currentPosition +=len;
	}
	return 0;
}