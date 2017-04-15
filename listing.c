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

#include "listing.h"

int listing(int argc, char *argv[])
{
	repositoryMetadata repo;
	fileMetadata files[MAX_NUMBER_OF_FILES];
	
	int vaultFile;

	if(argc != LISTING_NUMBER_OF_ARGUMENTS)
	{
		printf("Wrong number of arguments\n");
		return -1;
	}

	if(openFile(&vaultFile, argv[1], LISTING_FILE_OPEN_FLAG, 0) == -1)
		return -1;

	if(getRepositoryMetadata(vaultFile, &repo) == -1)
		return -1;
	if(getFilesMetadata(vaultFile, files) == -1)
		return -1;

	printVaultFileMetadata(repo,argv[1]);

	if(repo.files == 0)
		return 0;
	
	printFilesMetaData(files, repo.files);
	return 0;
}

void printVaultFileMetadata(repositoryMetadata repo, char* vaultFilePath)
{
	char timeString[20];
	char fileSize[10];
	char * vaultName = basename	(vaultFilePath);
	printf("%s\n", vaultName);

	strftime(timeString, 20, "%Y-%m-%d %H:%M:%S", localtime(&(repo.creationTime)));
	printf("Created at - %s\n",timeString);

	strftime(timeString, 20, "%Y-%m-%d %H:%M:%S", localtime(&(repo.modificationTime)));
	printf("Last modified at - %s\n",timeString);
	
	printsize(repo.size, fileSize);
	printf("Total file size - %s\n\n",fileSize);

	printf("Consist %d files", repo.files);
	if(repo.files == 0)
		printf("\n");
	else
		printf(":\n");
}

void printFilesMetaData(fileMetadata *files, int numberOfFiles)
{
	char timeString[20];
	char fileSize[10];
	for(int i=0; i<numberOfFiles && i<MAX_NUMBER_OF_FILES;i++)
	{
		fileMetadata *tempFile = files + i;
		strftime(timeString, 20, "%Y-%m-%d %H:%M:%S", localtime(&((*tempFile).creationTime)));
		printsize((*tempFile).size,fileSize);
		printf("%s\t%s\t%d\t%s\n", (*tempFile).name,fileSize,(*tempFile).permissions,timeString);

	}
}


//CRADIT - ssize_t to readable char* http://stackoverflow.com/questions/3898840/converting-a-number-of-bytes-into-a-file-size-in-c
//I changed it a bit to make it easier to use for me
void printsize(ssize_t size, char *str)
{                   
    static const char *SIZES[] = { "B", "kB", "MB", "GB" };
    size_t div = 0;
    size_t rem = 0;

    while (size >= 1024 && div < (sizeof SIZES / sizeof *SIZES)) {
        rem = (size % 1024);
        div++;   
        size /= 1024;
    }

    sprintf(str, "%.1f %s", (float)size + (float)rem / 1024.0, SIZES[div]);
}
//end cradit