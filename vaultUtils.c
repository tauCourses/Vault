#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <libgen.h>




#include "vaultUtils.h"

const off_t FILES_METADATA_OFFSET = sizeof(repositoryMetadata);
const off_t BLOCKS_OFFSET = sizeof(repositoryMetadata) + MAX_NUMBER_OF_FILES * sizeof(fileMetadata);

	

int openFile(int *file, char *address, int oflag, mode_t mode)
{
	char * name = basename	(address);
	if(mode == 0)
		*file = open(address, oflag);
	else
		*file = open(address, oflag, mode);  //open vault file
	if (*file < 0) 
	{
		printf( "Error opening %s file: %s\n", name,  strerror( errno ) );
		return -1;
    }
	
	return 0;
}

int getRepositoryMetadata(int vaultFile, repositoryMetadata *repository)
{
	if(lseek(vaultFile, REPOSITORY_METADATA_OFFSET, SEEK_SET) < 0) 
	{
		printf("Error seek in vault file: %s\n", strerror(errno));
		return -1;
	}	
	ssize_t len = read(vaultFile, (void *)(repository), sizeof(repositoryMetadata));

	if(len < 0) //check that the read call succeeded 
	{
		printf("Error reading from vault file: %s\n", strerror(errno));
		return -1;
	}
	else if(len != sizeof(repositoryMetadata))
	{
		printf("len - %zu", len);
		printf("Error - reading metadata failed\n");
		return -1;
	}
	return 0;
}

int saveRepositoryMetadata(int vaultFile, repositoryMetadata repository)
{
	if(lseek(vaultFile, REPOSITORY_METADATA_OFFSET, SEEK_SET) < 0) 
	{
		printf("Error seek in vault file: %s\n", strerror(errno));
		return -1;
	}

	if(write(vaultFile, (void *)(&repository), sizeof(repositoryMetadata)) < 0)
	{
		printf("Error write to vault file: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

int getFilesMetadata(int vaultFile, fileMetadata *filesMetadataArray)
{
	if(lseek(vaultFile, FILES_METADATA_OFFSET, SEEK_SET) < 0) 
	{
		printf("Error seek in vault file: %s\n", strerror(errno));
		return -1;
	}	
	for(int i=0; i<MAX_NUMBER_OF_FILES; i++)
	{
		fileMetadata *tempFile = filesMetadataArray + i;

		ssize_t len = read(vaultFile, (void *)(tempFile), sizeof(fileMetadata));

		if(len < 0) //check that the read call succeeded 
		{
			printf("Error reading from vault file: %s\n", strerror(errno));
			return -1;
		}
		else if(len != sizeof(fileMetadata))
		{
			printf("Error - reading files metadata failed\n");
			return -1;
		}
	}
	return 0;
}

int saveFilesMetadata(int vaultFile, const fileMetadata const *filesMetadataArray)
{
	if(lseek(vaultFile, FILES_METADATA_OFFSET, SEEK_SET) < 0) 
	{
		printf("Error seek in vault file: %s\n", strerror(errno));
		return -1;
	}	

	for(int i=0; i<MAX_NUMBER_OF_FILES; i++)
	{
		const fileMetadata *tempFile = filesMetadataArray + i;

		if(write(vaultFile, (void *)(tempFile), sizeof(fileMetadata)) < 0)
		{
			printf("Error write to vault file: %s\n", strerror(errno));
			return -1;
		}
	}
	return 0;
}

int saveFileLine(int vaultFile, fileMetadata newFile, int newFileNumber)
{
	if(lseek(vaultFile, FILES_METADATA_OFFSET + sizeof(fileMetadata) * newFileNumber, SEEK_SET) < 0) 
	{
		printf("Error seek in vault file: %s\n", strerror(errno));
		return -1;
	}	
	if(write(vaultFile, (void *)(&newFile), sizeof(fileMetadata)) < 0)
	{
		printf("Error write to vault file: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

int checkIfFileExist(char *fileName, const fileMetadata const *files)
{
	char * fileBaseName = basename(fileName);
	for(int i=0; i<MAX_NUMBER_OF_FILES; i++)
	{
		const fileMetadata const *tempFile = files + i;
		
		if(tempFile->creationTime == 0)
			break;
		if(strcmp(fileBaseName,tempFile->name) == 0)
			return i;
	}
	return -1;
}


int findFreeBlocks(const fileMetadata const *files, ssize_t size, blockMetadata **blocks, blockMetadata **unUsedSpaceBlock)
{
	chainedBlock *fragBlocks = NULL; //blocks in the middle of the data

	*unUsedSpaceBlock =  (blockMetadata*) malloc(sizeof(blockMetadata));
	if(*unUsedSpaceBlock == NULL)
		return -1;

	(*unUsedSpaceBlock)->offset = BLOCKS_OFFSET; //block starts at the begining of the blocks area
	(*unUsedSpaceBlock)->size = size - BLOCKS_OFFSET; //block size is all block area
	
	for(int i=0;i<MAX_NUMBER_OF_FILES;i++)
	{
		const fileMetadata const *tempFile = files + i;
		if(tempFile->creationTime == 0)
			break;
		for(int j=0;j<NUMBER_OF_BLOCKS_PER_FILE;j++)
		{
			if(tempFile->blocks[j].size == 0)
				break;
			if(addBlockConstraint(tempFile->blocks[j], *unUsedSpaceBlock, &fragBlocks) == -1)
				return -1;

		}
	}
	int numberOfBlocks = chainedToArray(fragBlocks,blocks);
	if(numberOfBlocks == -1)
		return -1;
	qsort(*blocks, numberOfBlocks, sizeof(blockMetadata), blockSizeComperator);

	#ifdef DEBUG_MODE
		printf("Memory state:\n");
		printf("Unused memory - start at %zu and in size of %zu\n", (*unUsedSpaceBlock)->offset, (*unUsedSpaceBlock)->size);
		printBlockArray(*blocks, numberOfBlocks);
	#endif

	return numberOfBlocks;
}

int addBlockConstraint(blockMetadata constraintBlock, blockMetadata *unUsedSpaceBlock, chainedBlock **fragBlocks)
{
	//printf("block offset %zu size %zu\n",constraintBlock.offset, constraintBlock.size);
	//printf("unUSed offset %zu size %zu\n",unUsedSpaceBlock->offset, unUsedSpaceBlock->size);
	if(constraintBlock.offset == unUsedSpaceBlock->offset)  //data block decrease remains size, not changing the fragBlocks
	{
		unUsedSpaceBlock->offset += constraintBlock.size;
		unUsedSpaceBlock->size -= constraintBlock.size;
	}
	else if(constraintBlock.offset > unUsedSpaceBlock->offset) //add a frag blcok to the end of the fragBlocks chain
	{
		chainedBlock* tempBlock = NULL;
		if(*fragBlocks == NULL)
		{
			tempBlock = (chainedBlock*) malloc(sizeof(chainedBlock));
			if(tempBlock == NULL)
				return -1;
			*fragBlocks = tempBlock;
		}
		else
		{
			tempBlock = *fragBlocks;
			while(tempBlock->next != NULL) tempBlock = tempBlock->next; //get last block;
			
			tempBlock->next = (chainedBlock*) malloc(sizeof(chainedBlock));
			if(tempBlock->next == NULL)
				return -1;

			tempBlock = tempBlock->next;
		}

		tempBlock->offset = unUsedSpaceBlock->offset;
		tempBlock->size = constraintBlock.offset - unUsedSpaceBlock->offset;
		//printf("new block offset %zu size %zu\n",tempBlock->offset, tempBlock->size);
	
		unUsedSpaceBlock->offset = constraintBlock.offset + constraintBlock.size;
		unUsedSpaceBlock->size -= (constraintBlock.size + tempBlock->size);
	}
	else //the block is somewhere inside fragBlocks
	{
		chainedBlock *tempBlock = *fragBlocks;
		chainedBlock *prev = NULL;
		while(constraintBlock.offset > tempBlock->offset + tempBlock->size )
		{
			prev = tempBlock;
			tempBlock = tempBlock->next; 
		}
		

		if(tempBlock->offset == constraintBlock.offset) //two blocks start at the same point
		{
			if(tempBlock->size > constraintBlock.size) //
			{
				tempBlock->offset += constraintBlock.size;
				tempBlock->size -= constraintBlock.size;
			}
			else
			{
				if(prev == NULL)
					*fragBlocks = tempBlock->next;
				else
					prev->next = tempBlock->next;

				free(tempBlock);
			}
		}
		else if(tempBlock->offset + tempBlock->size ==  constraintBlock.offset  + constraintBlock.size)
		{
			tempBlock->size -= constraintBlock.size;
		}
		else
		{
			chainedBlock *newBlock = (chainedBlock*) malloc(sizeof(chainedBlock));
			if(tempBlock == NULL)
				return -1;
			newBlock->next = tempBlock->next;
			tempBlock->next = newBlock;

			newBlock->offset = constraintBlock.offset + constraintBlock.size;
			newBlock->size = tempBlock->size - constraintBlock.size + tempBlock->offset - constraintBlock.offset;

			tempBlock->size = constraintBlock.offset - tempBlock->offset;
		}
	}
	return 0;
}

int chainedToArray(chainedBlock *chianed, blockMetadata **array)
{
	int numOfBlocks = 0;
	chainedBlock *tempBlock = chianed;
	
	while(tempBlock != NULL)
	{
		numOfBlocks++;
		tempBlock = tempBlock->next; //get last block;
	}
	if(numOfBlocks == 0)
		return 0;

	*array = (blockMetadata*) malloc(numOfBlocks * sizeof(blockMetadata));
	if(*array == NULL)
		return -1;

	tempBlock = chianed;
	for(int i=0;i<numOfBlocks;i++)
	{
		(*array)[i].size = tempBlock->size;
		(*array)[i].offset = tempBlock->offset;

		tempBlock = tempBlock->next;
	}
	return numOfBlocks;
}

int blockSizeComperator (const void * v1, const void * v2)
{
	const blockMetadata *p1 = (blockMetadata *)v1;
    const blockMetadata *p2 = (blockMetadata *)v2;
	
	if (p1->size < p2->size)
        return +1;
    else if (p1->size > p2->size)
        return -1;
    else if(p1->offset < p2->offset)
    	return +1;
    else if(p1->offset > p2->offset)
    	return -1;

    return 0;
}

void printBlockArray(blockMetadata *array, int numOfBlocks)
{
	for(int i=0;i<numOfBlocks;i++)
		printf("block %d - start at %zu and in size of %zu\n",i,array[i].offset,array[i].size);

	printf("\n");
}

int writeCharToFile(int file, off_t offset, char c, int repitations)
{
	char *arr = (char *) malloc(sizeof(char)*repitations);
	if(arr == NULL)
		return -1;

	for(int i=0; i < repitations; i++)
		arr[i] = c;

	if(offset!=0)
	{
		if(lseek(file, offset, SEEK_SET) < 0) 
		{
			printf("Error seek in vault file: %s\n", strerror(errno));
			return -1;
		}	
	}
	if(write(file, arr, repitations) < 0)
	{
		printf("Error write to vault file: %s\n", strerror(errno));
		return -1;
	}
	free(arr);
	return 0;
}

int getAllDataBlocks(fileMetadata *files, blockMetadata ***blocks)
{
	int numberOfBlocks = getNumberOfDataBlocks(files);

	blockMetadata **tempBlocks =  malloc(numberOfBlocks * sizeof(*tempBlocks));

	if(tempBlocks == NULL)
		return -1;

	int blockCounter = 0;
	for(int i=0;i<MAX_NUMBER_OF_FILES;i++)
	{
		fileMetadata *tempFile = files + i;
		if(tempFile->creationTime == 0)
			break;
		for(int j=0;j<NUMBER_OF_BLOCKS_PER_FILE;j++)
		{
			if(tempFile->blocks[j].size == 0)
				break;
			tempBlocks[blockCounter] = &(tempFile->blocks[j]);
			blockCounter++;

		}
	}
	
	qsort(tempBlocks, numberOfBlocks, sizeof(blockMetadata*), blockOffsetComperator);

	#ifdef DEBUG_MODE
		if(blockCounter>0)
		{
			printf("data blocks sort by offset:\n");
			printBlockPointer(tempBlocks, numberOfBlocks);
		}
		else
			printf("not data blocks found, empty file\n");
	#endif
		
	*blocks = tempBlocks;
	return numberOfBlocks;
}

int getNumberOfDataBlocks(const fileMetadata const *files)
{
	int numberOfBlocks = 0;
	for(int i=0;i<MAX_NUMBER_OF_FILES;i++)
	{
		const fileMetadata const *tempFile = files + i;
		if(tempFile->creationTime == 0)
			break;
		for(int j=0;j<NUMBER_OF_BLOCKS_PER_FILE;j++)
		{
			if(tempFile->blocks[j].size == 0)
				break;
			numberOfBlocks++;

		}
	}
	
	return numberOfBlocks;
}

int blockOffsetComperator(const void * v1, const void * v2)
{
	const blockMetadata *p1 = *(blockMetadata * const *)v1;
    const blockMetadata *p2 = *(blockMetadata * const *)v2;
	
	return p1->offset - p2->offset;
	return 1;
}

void printBlockPointer(blockMetadata **blocks, int numberOfBlocks)
{
	for(int i=0;i<numberOfBlocks;i++)
		printf("%d\t%zu\t%zu\n",i,blocks[i]->offset, blocks[i]->size);
	printf("\n");
}