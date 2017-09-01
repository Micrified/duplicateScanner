/*
********************************************************************************
*                                
* Filename     : duplicateTracker.c
* Programmer(s): Owatch
* Created      : 2017/08/29
* Description  : Hashes and tracks given filenames.
********************************************************************************
*/

#include "duplicateTracker.h"

/*
 ******************************************************************************
 *                        Symbolic Constants & Structures
 ******************************************************************************
 */

/* Printing format for a file node */
#define FPRINT_FORMAT   "\t%d:\t%-32s%-32s\n"

/* Structure representing a file */
typedef struct file {
    char *filePath;
    time_t modified;
} File;

/* Structure representing a linked list node */
struct node {
    File file;
    struct node *next;
};

/*
 ******************************************************************************
 *                             Auxillary Functions
 ******************************************************************************
 */

/* Allocates and initializes a new list node */
static struct node *newNode (const char *filePath, const time_t modified) {
    struct node *n;

    // Fail if node can't be allocated.
    if ((n = malloc(sizeof(struct node))) == NULL) {
        return NULL;
    }

    // Fail if path field can't be allocated.
    if (filePath == NULL || (n->file.filePath = malloc(MAX_PATH)) == NULL) {
        return NULL;
    }

    // Assign fields.
    strncpy(n->file.filePath, filePath, MAX_PATH);
    n->file.modified = modified;
    n->next = NULL;

    return n;
}

/* Recursively free's the given list node and it's linked nodes (Warning). */
static void freeNode (struct node *n) {
    if (n == NULL) {
        return;
    }
    free(n->next);
    free(n->file.filePath);
    free(n);
}

/* Returns the file name of a file from a given file path */
static char *fileName (char *filePath) {
    char *copy, *fileName = filePath;

    if (filePath == NULL) {
        return "NUll";
    }

    for (copy = filePath; *copy != '\0'; copy++) {
        if (*copy == '/') {
            fileName = copy + 1;
        }
    }

    return fileName;
}

/*
 ******************************************************************************
 *                             Hash Table Functions
 ******************************************************************************
 */

 #define TBL_SIZE       512000

 #define FNV_PRIME      16777619

 #define FNV_OFFSET     2166136261

/* Computes FNV-1a hash for the provided key. Filenames should be ASCII encoded */
long hash (const char *key) {
    long hash = FNV_OFFSET;

    do {
        hash = hash ^ (*key);
        hash *= FNV_PRIME;
    } while (*++key != '\0');

    hash %= TBL_SIZE;
    return hash < 0 ? -hash : hash;
}

/*
 ******************************************************************************
 *                             File Table Functions.
 ******************************************************************************
 */

/* The internal file table */
static struct node **fileTable;

/* The file count */
static long fileCount;

/* Inserts a node into fileTable. Signals error with nonzero value */
static int insertNode (struct node *n) {
    int index = hash(fileName(n->file.filePath));
    struct node *next, *head;

    // Don't insert into unallocated table.
    if (fileTable == NULL) {
        return 1;
    }
    //fprintf(stdout, "Inserting new node at index %d/%d\n", index, TBL_SIZE);
    // Increment file count
    fileCount++;

    head = fileTable[index];

    // Put at head of list if no node yet, or newer than newest node.
    if (head == NULL || difftime(n->file.modified, head->file.modified) > 0) {
        fileTable[index] = n;
        n->next = head;
        return 0;
    }

    // Parse list until reached end or 'next' is an older node.
    for (next = head->next; 
         next != NULL && difftime(n->file.modified, next->file.modified) <= 0;
         head = next, next = head->next)
        ;
    
    head->next = n;
    n->next = next;

    return 0;
}

/*
 ******************************************************************************
 *                             Private Functions
 ******************************************************************************
 */

/* Print's a file list. */
static void printFileChain (struct node *n) {
    int i = 1, count = 1;

    // Do not print empty lists.
    if (n == NULL) {
        return;
    }

    // Count files.
    for (struct node *copy = n->next; copy != NULL; count++, copy = copy->next);

    // Output file details.
    fprintf(stdout, "FILE (x%d): %-64s\n", count, fileName(n->file.filePath));
    do {
        char *timeString =  ctime(&(n->file.modified));
        timeString[strlen(timeString) - 1] = '\0';
        fprintf(stdout, FPRINT_FORMAT, i++, timeString, n->file.filePath);
    } while ((n = n->next) != NULL);

    // Output final newline buffer.
    putchar('\n');
} 

/*
 ******************************************************************************
 *                             Public Functions
 ******************************************************************************
 */

/* Hashes and logs the given file details. Signals error with nonzero return */
int trackFile (const char *fileName, const time_t modified) {
    struct node *n;

    // Return nonzero error if allocation of node failed. 
    if ((n = newNode(fileName, modified)) == NULL) {
        return 1;
    }

    //fprintf(stdout, "inserting %s...\n", fileName);
    return insertNode(n);
}
 
/* Initializes the internal file table */
int initializeFileTable (void) {
    
    // Ensure all are initialized to NULL.
    if ((fileTable = calloc(TBL_SIZE, sizeof(struct node *))) == NULL) {
        return 1;
    }
    return 0;
}
 
/* Prints all duplicate files logged in the filetable by desc modified date */
void printFileTable (void) {
    
    // Don't print an uninitialized fileTable.
    if (fileTable == NULL) {
        fprintf(stdout, "FileTable is NULL!\n");
        return;
    }

    // Print each list of duplicate files.
    for (int i = 0; i < TBL_SIZE; i++) {
        printFileChain(fileTable[i]);
    }
}

 /* Searches the file table for a particular file name. Then prints results */
 void findFile (const char *fileName) {
    int index;
    // Don't search if the fileTable is uninitialized.
    if (fileTable == NULL) {
        fprintf(stderr, "Error: File Table is uninitialized!\n");
        return;
    }

    // Compute hash, search table.
    if (fileTable[(index = hash(fileName))] == NULL) {
        fprintf(stdout, "Sorry, no match found!\n");
    } else {
        printFileChain(fileTable[index]);
    }
 }

/* Returns the total number of files in the file table */
long getFileCount (void) {
    return fileCount;
}
 
/* Free's the internal file table (and all files) */
int freeFileTable (void) {

    // Do not free if fileTable already NULL.
    if (fileTable == NULL) {
        return 1;
    }

    // Free all nodes.
    for (int i = 0; i < TBL_SIZE; i++) {
        freeNode(fileTable[i]);
    }

    // Free table, NULL pointer.
    free(fileTable);
    fileTable = NULL;

    return 0;
}
