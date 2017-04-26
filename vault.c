#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <dirent.h>
#include <fcntl.h> // for open flags

#include "vaultUtils.h" //here is all the important structures and constances

#include "initVault.h"
#include "insertFile.h"
#include "listing.h"
#include "remove.h"
#include "fetchFile.h"
#include "defrag.h"
#include "status.h"


#define _FILE_OFFSET_BITS 64

typedef enum {
	INIT_OPERATION, 
	LIST_OPERATION, 
	INSERT_OPERATION, 
	REMOVE_OPERATION, 
	FETCH_OPERATION, 
	DEFRAG_OPERATION, 
	STATUS_OPERATION, 
	UNKNOWN_OPERATION
	} OPERATIONS_TYPES;

typedef int (*operationFunction)(int argc, char *argv[]); //function pointer definition

operationFunction operationFunctionArray[] = {	initVault, 
												listing, 
												insertFile, 
												removeFile,
												fetchFile,
												defragVault,
												getStatus}; //an array of function pointers for each operation

OPERATIONS_TYPES getOperation(char* str);

int main( int argc, char *argv[] )  
{
	struct timeval start, end;
  	long seconds, useconds;
  	double mtime;
  	int value = -1; 
	gettimeofday(&start, NULL);

	if(argc>=3) //if there are at least 3 arguments
	{
		OPERATIONS_TYPES operation = getOperation(argv[2]); //get operation

		if(operation != UNKNOWN_OPERATION) //if it's a known operation 
		{
			value = (*operationFunctionArray[operation])(argc, argv);
			if(value == -1)
				printf("vault program failed\n");
		}
		else
			printf("Unknown operation\n");
	}
	else
		printf("Wrong number of arguments\n");
	
	


	gettimeofday(&end, NULL);

    seconds  = end.tv_sec  - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    
    mtime = ((seconds) * 1000 + useconds/1000.0);
    printf("Elapsed time: %.3f milliseconds\n", mtime);
	return value;
}

OPERATIONS_TYPES getOperation(char* str)
{
	//CRADIT http://stackoverflow.com/questions/2661766/c-convert-a-mixed-case-string-to-all-lower-case
	for (char *ch = str; *ch; ++ch) *ch = tolower(*ch);
	//END CRADIT!!! 

	if(strcmp(str,"init") == 0)
		return INIT_OPERATION;
	else if(strcmp(str,"list") == 0)
		return LIST_OPERATION;
	else if(strcmp(str,"add") == 0)
		return INSERT_OPERATION;
	else if(strcmp(str, "rm") == 0)
		return REMOVE_OPERATION;
	else if(strcmp(str, "fetch") == 0)
		return FETCH_OPERATION;
	else if(strcmp(str, "defrag") == 0)
		return DEFRAG_OPERATION;
	else if(strcmp(str, "status") == 0)
		return STATUS_OPERATION;

	return UNKNOWN_OPERATION;
}
