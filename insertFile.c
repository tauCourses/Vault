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

#include "insertFile.h"

int insertFile(int argc, char *argv[])
{
	repositoryMetadata repo;
	fileMetadata files[MAX_NUMBER_OF_FILES], newFile;
	struct stat requestedFileStatus;
	int vaultFile;
	int insertFile;

	blockMetadata *fragBlocks = NULL , *unUsed = NULL, *choosen = NULL;
	int numberOfFragBlocks, numberOfChosenBlocks;
	
	struct timeval time;
	char * newFileName = basename(argv[3]);
	int newFileNumber;

	if(argc != INSERT_NUMBER_OF_ARGUMENTS)
	{
		printf("Wrong number of arguments\n");
		return -1;
	}

	if(openFile(&vaultFile, argv[1], INSERT_VAULT_FILE_OPEN_FLAG, 0) == -1)
		return -1;

	if(getRepositoryMetadata(vaultFile, &repo) == -1)
		return -1;

	if(getFilesMetadata(vaultFile, files) == -1)
		return -1;
	
	if(repo.files == MAX_NUMBER_OF_FILES)
	{
		printf("File system is full, can't insert a new file");
		return -1;
	}
	
	
	if(openFile(&insertFile, argv[3], INSERT_NEW_OPEN_FLAG, VAULT_MODE_T) == -1)
		return -1;

	if(checkIfFileExist(argv[3], files) != -1)
	{
		printf("File exist or there is another file with the same name in the vault\n");
		return -1;
	}

	if(stat(argv[3], &requestedFileStatus) != 0) 
	{
		printf("ERROR - can't excess to requested file - %s\n", strerror(errno));
		return -1;
	}

	if(gettimeofday(&time, NULL) != 0)
	{
		printf("Failed to get current time: %s\n", strerror( errno ) );
		return -1;
	}
	numberOfFragBlocks = findFreeBlocks(files, repo.size, &fragBlocks, &unUsed);
	if(numberOfFragBlocks == -1)
		return -1;

	
	numberOfChosenBlocks = findBlocksToSaveFile(requestedFileStatus.st_size, fragBlocks, numberOfFragBlocks, unUsed, &choosen);
	if(numberOfChosenBlocks == -1)
		return -1;

	#ifdef DEBUG_MODE
		printf("\nchosen blcoks:\n");
		printBlockArray(choosen, numberOfChosenBlocks);
	#endif
	
	if(copyFileToBlocks(vaultFile, insertFile, requestedFileStatus.st_size, choosen, numberOfChosenBlocks) == -1)
		return -1;

	newFileNumber = createNewFile(&newFile, files, newFileName, requestedFileStatus.st_size, requestedFileStatus.st_mode,
						time.tv_sec, choosen, numberOfChosenBlocks);
	if(newFileNumber == -1)
		return -1;

	if(saveFileLine(vaultFile, newFile, newFileNumber) == -1)
		return -1;

	repo.files++;
	repo.modificationTime = time.tv_sec;
	
	if(saveRepositoryMetadata(vaultFile, repo) == -1)
		return -1;

	free(choosen);
	free(fragBlocks);
	free(unUsed);

	if(close(vaultFile) < 0)
	{
		printf("ERROR: unable to close file %s\n", strerror(errno));
		return -1;
	}
	if(close(insertFile) < 0)
	{
		printf("ERROR: unable to close file %s\n", strerror(errno));
		return -1;
	}

	return 0;
}

int findBlocksToSaveFile(ssize_t fileSize, blockMetadata *availableBlocks, int numberOfBlocks, blockMetadata *unUsed, blockMetadata **chosenBlocks)
{
	size_t chosenBlocksSize = 0; 
	size_t fileRemainSize = fileSize;
	int numOfChosenBlocks = 0;
	int topLimit = numberOfBlocks;  
	
	for(int i=0; i<NUMBER_OF_BLOCKS_PER_FILE && i<numberOfBlocks; i++)
	{
		numOfChosenBlocks++;
		chosenBlocksSize += availableBlocks[i].size;
		if(chosenBlocksSize - numOfChosenBlocks*DELIMETER_SIZE*2 >= fileSize)
			break;
	}
	#ifdef DEBUG_MODE
		printf("size needed to be allocated - %zu\n", fileSize);
	#endif
	
	if(chosenBlocksSize - numOfChosenBlocks*DELIMETER_SIZE*2 < fileSize) //need to use unUsed space
	{
		#ifdef DEBUG_MODE
			printf("need to use unUsed memory\n");
		#endif
		if(numOfChosenBlocks == NUMBER_OF_BLOCKS_PER_FILE )
		{
			chosenBlocksSize -= availableBlocks[2].size; //remove the smallest block
			numOfChosenBlocks--;
		}
		numOfChosenBlocks++; //we'll add one more block from the unUsed area

		if(chosenBlocksSize + unUsed->size - numOfChosenBlocks*DELIMETER_SIZE*2 >= fileSize)
		{
			*chosenBlocks = (blockMetadata*) malloc(numOfChosenBlocks * sizeof(blockMetadata));
			if(*chosenBlocks == NULL)
				return -1;

			for(int i=0;i<numOfChosenBlocks-1; i++)
			{
				(*chosenBlocks)[i].offset = availableBlocks[i].offset;
				(*chosenBlocks)[i].size = availableBlocks[i].size;
				fileRemainSize -= (*chosenBlocks)[i].size - DELIMETER_SIZE*2;
			}

			(*chosenBlocks)[numOfChosenBlocks-1].offset = unUsed->offset;
			(*chosenBlocks)[numOfChosenBlocks-1].size = fileRemainSize + DELIMETER_SIZE*2;

			

			return numOfChosenBlocks;
		}
		else //not enough space to store the file in 3 blocks
		{
			printf("Not enought memory to save the file\n");
			return -1;
		}
	}
	
	//if the file can be stored in the gaps between used data:
	*chosenBlocks = (blockMetadata*) malloc(numOfChosenBlocks * sizeof(blockMetadata));
	if(*chosenBlocks == NULL)
		return -1;

	for(int i = numOfChosenBlocks-1; i>=0; i--) //prefer small blocks rathar than big blocks
	{
		int j = i;
		//check if a smaller block is:
		//1.usable block - the block exist and it's not a used block. 
		//2.still have enought space
		
		while(j<topLimit && 
			chosenBlocksSize - availableBlocks[i].size + availableBlocks[j].size - numOfChosenBlocks*DELIMETER_SIZE*2 >= fileSize) 
				j++;
		
		j--; //j-1 is the last number that fill all the requirments
		
		(*chosenBlocks)[numOfChosenBlocks-i-1].offset = availableBlocks[j].offset;
		(*chosenBlocks)[numOfChosenBlocks-i-1].size = availableBlocks[j].size;
		chosenBlocksSize = chosenBlocksSize - availableBlocks[i].size + availableBlocks[j].size;
		if(i==0) //in the first
			(*chosenBlocks)[numOfChosenBlocks-i-1].size = fileRemainSize + DELIMETER_SIZE*2;
		
		fileRemainSize -= ((*chosenBlocks)[numOfChosenBlocks-i-1].size - DELIMETER_SIZE*2);

		#ifdef DEBUG_MODE
			printf("Block offset - %zu\t Block size- %zu",   
				(*chosenBlocks)[numOfChosenBlocks-i-1].offset, 
				(*chosenBlocks)[numOfChosenBlocks-i-1].size);
			if(fileRemainSize>0)
				printf("\tstill need to fill %zu\n",fileRemainSize);
			else
				printf("\n");
		#endif

		topLimit = j;
	}

	return numOfChosenBlocks;
}

int createNewFile(fileMetadata *newFile, fileMetadata *files, char * newFileName, off_t st_size, mode_t st_mode, time_t time, blockMetadata *blocks, int numberOfBlocks)
{
	strcpy(newFile->name, newFileName);
	newFile->size = st_size;
	newFile->permissions = st_mode;
	newFile->creationTime = time;
	
	for(int i=0;i<numberOfBlocks;i++)
	{
		newFile->blocks[i].size = blocks[i].size;
		newFile->blocks[i].offset = blocks[i].offset;
	}

	for(int i=0; i<MAX_NUMBER_OF_FILES; i++)
	{
		const fileMetadata const *tempFile = files + i;
		if(tempFile->creationTime == 0)
			return i;
	}
	printf("Error - vault is full\n");
	return -1;
}

int copyFileToBlocks(int vaultFile, int insertFile, size_t fileSize, blockMetadata *blocks, int numberOfBlocks)
{
	off_t offset = 0;
	for(int i=0; i<numberOfBlocks; i++)
	{	
		if(writeCharToFile(vaultFile, blocks[i].offset, START_BLOCK_DELIMITER, DELIMETER_SIZE) == -1)
			return -1;

		off_t temp = copyBlockToVault(vaultFile, insertFile, blocks[i].offset, offset, blocks[i].size-DELIMETER_SIZE*2);
		if(temp == -1)
			return -1;

		if(writeCharToFile(vaultFile, 0, END_BLOCK_DELIMITER, DELIMETER_SIZE) == -1)
			return -1;

		offset += temp;
	}
	return 0;
}

off_t copyBlockToVault(int vaultFile, int insertFile, off_t vaultFileOffset, off_t insertFileOffset, size_t size)
{
	char buffer[FILE_BUFFER_SIZE];
	size_t readingSize = size;
	while(readingSize > 0) //while more bytes need to be processed
	{
		ssize_t len;
		if(readingSize<FILE_BUFFER_SIZE) //read the minimum between readingSize and bufferSize
			len = readAll(insertFile, buffer, readingSize);
		else
			len = readAll(insertFile, buffer, FILE_BUFFER_SIZE);

		if(len < 0) //check that the read call succeeded 
			return -1;
		
		if(writeAll(vaultFile, buffer, len) < 0)
			return -1;
			
		readingSize -= len;
	}
	return size;
}


	