#ifndef VAULT_UTILS_HEADER_FILE
#define VAULT_UTILS_HEADER_FILE

#define FILE_BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 256
#define MAX_NUMBER_OF_FILES 100

#define REPOSITORY_METADATA_OFFSET 0
//#define FILES_METADATA_OFFSET sizeof(repositoryMetadata)
//#define BLOCKS_OFFSET FILES_METADATA_OFFSET + MAX_NUMBER_OF_FILES * sizeof(fileMetadata)
#define NUMBER_OF_BLOCKS_PER_FILE 3

#define DELIMETER_SIZE 8 // the num signes <<<<<< and >>>>> for the begining and the end of a block
#define START_BLOCK_DELIMITER '<'
#define END_BLOCK_DELIMITER '>'

typedef struct 
{
	ssize_t size;
	time_t creationTime;
	time_t modificationTime;
	short files;
} repositoryMetadata; 

typedef struct 
{
	off_t offset;
	ssize_t size;
} blockMetadata;

typedef struct 
{
	char name[FILE_NAME_MAX_SIZE];
	ssize_t size;
	mode_t permissions;
	time_t creationTime;
	blockMetadata blocks[NUMBER_OF_BLOCKS_PER_FILE];
} fileMetadata;

typedef struct _chainedBlock chainedBlock;

struct _chainedBlock //used to find all avialable blocks
{
	off_t offset;
	ssize_t size;
	chainedBlock* next;
};

extern const off_t FILES_METADATA_OFFSET;
extern const off_t BLOCKS_OFFSET;


int openFile(int *file, char *address, int oflag);

int getRepositoryMetadata(int vaultFile, repositoryMetadata *repository);
int saveRepositoryMetadata(int vaultFile, repositoryMetadata repository);

int getFilesMetadata(int vaultFile, fileMetadata *filesMetadataArray);
int saveFilesMetadata(int vaultFile, const fileMetadata const *filesMetadataArray);
int saveFileLine(int vaultFile, fileMetadata newFile, int newFileNumber);

int checkIfFileExist(char *fileName, const fileMetadata const *files);

int findFreeBlocks(const fileMetadata const *files, ssize_t size, blockMetadata **blocks, blockMetadata **unUsedSpaceBlock);
int addBlockConstraint(blockMetadata constraintBlock, blockMetadata *unUsedSpaceBlock, chainedBlock **fragBlocks);
int chainedToArray(chainedBlock *chianed, blockMetadata **array);
int blockComperator (const void * v1, const void * v2);
void printBlockArray(blockMetadata *array, int numOfBlocks);

int writeCharToFile(int file, off_t offset, char c, int repitations);

#endif