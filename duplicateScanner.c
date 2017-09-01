/*
********************************************************************************
*                                
* Filename     : duplicateScanner
* Programmer(s): Owatch
* Created      : 2017/08/29
* Description  : Adaptation of K&R fsize, recursively lists file modified dates.
********************************************************************************
*/

#include "duplicateTracker.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <dirent.h>
#include <ctype.h>

/*
 ******************************************************************************
 *                        Symbolic Constants & Structures
 ******************************************************************************
 */

/* Program name */
#define PRGM_NAME   "duplicateScanner"

/* Program usage */
#define PRGM_USE    "(Type/Drag) in directories to scan delimited by spaces.\n"\
                    "\tI.E: ./duplicateScanner <dir1> <dir2> ... <dirN>\n"

/* Program options */
#define PRGM_SRH    's'
#define PRGM_ALL    'a'
#define PRGM_EXT    'q'

#define PRGM_OPT    "\n- Search duplicates by name: s\n"\
                    "- Print file table contents: a\n"\
                    "- Quit (cleanly)           : q\n"

/* Directory Entry: Filename and inode */
typedef struct {
    long index;
    char fileName[NAME_MAX + 1];
} DirEntry;


/* Forward declarations (Prototypes) */
void scanDirectory (const char *, void (*)(const char *));



/*
 ******************************************************************************
 *                           System Dependent Functions
 ******************************************************************************
 */

/* Allocates a DIR object for readDirectory calls (System dependent). */
DIR *openDirectory (const char *directoryName) {
    return opendir(directoryName);
}

/* Frees a DIR object (System dependent). */
void closeDirectory (DIR *directory) {
    if (directory == NULL) {
        return;
    } else {
        closedir(directory);
    }
}

/* Returns consecutive directory-entries from a directory stream */
DirEntry *readDirectoryEntry (DIR *directory) {
    struct dirent *entryBuffer; // Standard buffer size of entry in DIR.
    static DirEntry entry;

    // Repeatedly write entries to the buffer while the byte count aligns.
    while ((entryBuffer = readdir(directory)) != NULL) {

        // If the entry is unused, continue.
        if (entryBuffer->d_ino == 0) {
            continue;
        }

        entry.index = entryBuffer->d_ino;
        strncpy(entry.fileName, entryBuffer->d_name, NAME_MAX);
        entry.fileName[NAME_MAX] = '\0';
        return &entry;
    }

    return NULL;
}

/*
 ******************************************************************************
 *                          System Independent Functions
 ******************************************************************************
 */

/* Prints last modified date of file to standard out. If dir, dir is walked. */
void scanFile (const char *fileName) {
    struct stat statBuffer; // For use with stat()

    // System call to stat to get file info.
    if (stat(fileName, &statBuffer) == -1) {
        fprintf(stderr, "Error: Can't access file %s! -Ignoring-\n", fileName);
        return;
    }

    // If directory, recursively apply scanFile to contents.
    if ((statBuffer.st_mode & S_IFMT) == S_IFDIR) {
        fprintf(stdout, "\tNote: Scanning directory %s\n", fileName);
        scanDirectory(fileName, scanFile);
    } else {
        time_t modified = statBuffer.st_mtime;

        // Track file in file table.
        if (trackFile(fileName, modified)) {
            fprintf(stderr, "Error: File couldn't be logged! -Ignoring-\n");
        }
    }
}

/* Applies the given function 'f' to all files within the given directory */
void scanDirectory (const char *directoryName, void (*f)(const char *)) {
    char pathName[MAX_PATH];
    DirEntry *entry;
    DIR *directory;

    // Open the directory.
    if ((directory = openDirectory(directoryName)) == NULL) {
        fprintf(stderr, "Error: Can't access directory %s! -Ignoring-\n", 
        directoryName);
        return;
    }

    // Scan the directory contents.
    while ((entry = readDirectoryEntry(directory)) != NULL) {
        char *fileName = entry->fileName;

        // Ignore self, parent.
        if (strcmp(fileName, ".") == 0 || strcmp(fileName, "..") == 0) {
            continue;
        }

        // If path isn't too long, append new file path to buffer and scan it.
        if (strlen(directoryName) + strlen(fileName) + 2 > MAX_PATH) {
            fprintf(stderr, "Error: %s filepath too long! -Ignoring-\n", fileName);
        } else {
            sprintf(pathName, "%s/%s", directoryName, fileName);
            (*f)(pathName);
        }
    }

    // Close the directory.
    closeDirectory(directory);
}

/* Main: Scans current directory if no arguments given. Else scans arguments */
int main (int argc, const char *argv[]) {
    char option, fileName[NAME_MAX];

    // Ensure that at least one directory has been specified.
    if (argc == 1) {
        fprintf(stdout, "%s: %s", PRGM_NAME, PRGM_USE);
        return -1;
    }

    // Attempt to allocate the file table.
    if (initializeFileTable()) {
        fprintf(stderr, "Error: Couldn't start up the file table!\n");
        return -1;
    }

    // Scan all given directories.
    while (--argc > 0) {
        fprintf(stdout, "%s: Scanning top-level directory %s\n", PRGM_NAME, *(argv + 1));
        scanFile (*++argv);
    }

    // Output results, prompt to search/dump contents/exit.
    fprintf(stdout, "%s: Finished scanning (%ld files found).\n", PRGM_NAME, getFileCount());
    do {
        fprintf(stdout, "%s:", PRGM_OPT);
        scanf("\n%c", &option);

        if (option == PRGM_ALL) {
            printFileTable();
        }

        if (option == PRGM_SRH) {
            fprintf(stdout, "\nName: ");
            scanf("%255s", fileName);
            fprintf(stdout, "\nSearching for %s\n", fileName);
            findFile(fileName);
        }
    } while (option != PRGM_EXT);


    // Clean up.
    if (freeFileTable()) {
        fprintf(stderr, "Error: Problem free'ing the file table!\n");
    }

    return 0;
}
