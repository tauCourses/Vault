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

#include "fetchFile.h"

int fetchFile(int argc, char *argv[])
{
	repositoryMetadata repo;
	fileMetadata files[MAX_NUMBER_OF_FILES];
	
	int vaultFile, fetchFile, fetchFileNumber; 

	if(argc != FETCH_NUMBER_OF_ARGUMENTS)
	{
		printf("Wrong number of arguments\n");
		return -1;
	}

	if(openFile(&vaultFile, argv[1], FETCH_VAULT_FILE_OPEN_FLAG, 0) == -1)
		return -1;

	if(getRepositoryMetadata(vaultFile, &repo) == -1)
		return -1;

	if(getFilesMetadata(vaultFile, files) == -1)
		return -1;

	fetchFileNumber = checkIfFileExist(argv[3], files);
	if(fetchFileNumber == -1)
	{
		printf("File not exist\n");
		return -1;
	}

	if(openFile(&fetchFile, argv[3], FETCH_NEW_FILE_OPEN_FLAG, files[fetchFileNumber].permissions) == -1) //todo - permmisions!
		return -1;

	if(copyFile(vaultFile, fetchFile, files[fetchFileNumber]) == -1)
		return -1;

	if(close(vaultFile) < 0)
	{
		printf("ERROR: unable to close file %s\n", strerror(errno));
		return -1;
	}
	
	if(close(fetchFile) < 0)
	{
		printf("ERROR: unable to close file %s\n", strerror(errno));
		return -1;
	}
	return 0;
}
int copyFile(int vaultFile, int fetchFile, fileMetadata file)
{
	for(int i=0;i<NUMBER_OF_BLOCKS_PER_FILE;i++)
	{
		if(file.blocks[i].size == 0)
			break;
		copyBlock(vaultFile, fetchFile, file.blocks[i]);
	}
	return 0;
}
int copyBlock(int vaultFile, int fetchFile, blockMetadata block)
{
	char buffer[FILE_BUFFER_SIZE];
	size_t readingSize = block.size-DELIMETER_SIZE*2;
	if(lseek(vaultFile, block.offset + DELIMETER_SIZE, SEEK_SET) < 0) 
	{
		printf("Error seek in vault file: %s\n", strerror(errno));
		return -1;
	}	
	while(readingSize > 0) //while more bytes need to be processed
	{
		ssize_t len;
		if(readingSize<FILE_BUFFER_SIZE) //read the minimum between readingSize and bufferSize
			len = readAll(vaultFile, buffer, readingSize);
		else
			len = readAll(vaultFile, buffer, FILE_BUFFER_SIZE);

		if(len < 0) //check that the read call succeeded 
			return -1;
		
		if(writeAll(fetchFile, buffer, len))
			return -1;
		
		readingSize -= len;
	}
	return 0;
}
