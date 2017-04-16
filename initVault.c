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

#include "initVault.h"
#include "listing.h"

int initVault(int argc, char *argv[])
{
	repositoryMetadata repo;
	fileMetadata *files = (fileMetadata*) malloc(MAX_NUMBER_OF_FILES*sizeof(fileMetadata));
	size_t fileSize;
	int vaultFile;

	if(files == NULL)
		return -1;

	if(argc != INIT_NUMBER_OF_ARGUMENTS)
	{
		printf("Wrong number of arguments\n");
		return -1;
	}

	if(parseDataAmount(argv[3],&fileSize) == -1)
		return -1;

	if(fileSize <= BLOCKS_OFFSET)
	{
		printf("minimum size %zu, size allocate %zu\n", BLOCKS_OFFSET, fileSize);
		printf("Size is too small, please create a bigger file\n");
		return -1;
	}
	if(openFile(&vaultFile, argv[1], INIT_FILE_OPEN_FLAG,VAULT_MODE_T) == -1)
		return -1;
	

	if(setFileSize(vaultFile, fileSize) == -1)
		return -1;

	if(setDefaultValues(&repo, files, fileSize) == -1)
		return -1;
	
	if(saveRepositoryMetadata(vaultFile,repo) == -1)
		return -1;

	if(saveFilesMetadata(vaultFile,files) == -1)
		return -1;

	free(files);
	if(close(vaultFile) < 0)
	{
		printf("ERROR: unable to close file %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

int setDefaultValues(repositoryMetadata *repo, fileMetadata *files, size_t size)
{
	struct timeval time;
	if(gettimeofday(&time, NULL) != 0)
	{
		printf("Failed to get current time: %s\n", strerror( errno ) );
		return -1;
	}

	(*repo).size = size;
	(*repo).creationTime = time.tv_sec;
	(*repo).modificationTime = time.tv_sec;
	(*repo).files = 0; //empty repo

	for(int i=0;i<MAX_NUMBER_OF_FILES;i++)
	{
		fileMetadata *tempFile = &files[i];
	
		strcpy( (*tempFile).name, "" );
		(*tempFile).size = 0;
		(*tempFile).permissions = 0;
		(*tempFile).creationTime = 0;
		for(int j=0;j<NUMBER_OF_BLOCKS_PER_FILE;j++)
		{
			blockMetadata *tempBlock = &(*tempFile).blocks[j];
			(*tempBlock).offset = 0;
			(*tempBlock).size = 0;
		} 
	}
	return 0;
}
int setFileSize(int file, size_t size)
{
	if(lseek(file, size-1, SEEK_SET) < 0) 
	{
		printf("Error write to vault file: %s\n", strerror(errno));
		return -1;
	}	
	
	if(write(file, "\0", 1) < 0)
	{
		printf("Error write to vault file: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

int parseDataAmount(char* arg, size_t *fileSize)
{
	char ch;
	
	if(sscanf(arg,"%zd%c",fileSize,&ch) == EOF) //parse 
	{
		printf("Failed to parse file size\n");
		return -1;
	}
	
	switch(ch) 
	{
		case 'B': //Bytes
			break;
		case 'K': //Kilo bytes
			*fileSize *= 1024;
			break;
		case 'M': //Mega bytes
			*fileSize *= 1024*1024;
			break;
		case 'G': //Giga bytes
			*fileSize *= 1024*1024*1024;
			break;
		default: //unknow char
			printf("Failed to parse file size\n");
			return -1;
	}
	
	return 0;
}