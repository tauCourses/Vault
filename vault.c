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

#include "vaultUtils.h"
#include "initVault.h"
#include "insertFile.h"
#include "listing.h"
#include "remove.h"


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

operationFunction operationFunctionArray[] = {initVault, listing, insertFile, removeFile}; //an array of function pointers for each operation

OPERATIONS_TYPES getOperation(char* str);

int main( int argc, char *argv[] )  
{
	if(argc<3)
	{
		printf("Wrong number of arguments\n");
		return -1;
	}
	OPERATIONS_TYPES operation = getOperation(argv[2]);

	if(operation == UNKNOWN_OPERATION)
	{
		printf("Unknown operation\n");
		return -1;
	}
	return (*operationFunctionArray[operation])(argc, argv);
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