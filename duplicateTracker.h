/*
********************************************************************************
*                                
* Filename     : duplicateTracker.h
* Programmer(s): Owatch
* Created      : 2017/08/29
* Description  : Hashes and tracks given filenames.
********************************************************************************
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#if !defined(duplicateTracker_h)
#define duplicateTracker_h

/*
 ******************************************************************************
 *                        Symbolic Constants & Structures
 ******************************************************************************
 */

/* The maximum length of a filename */
#define NAME_MAX    255

/* The maximum length of a filepath */
#define MAX_PATH    4096

/*
 ******************************************************************************
 *                                  Prototypes
 ******************************************************************************
 */

 /* Hashes and logs the given file details */
 int trackFile (const char *filePath, const time_t modified);

 /* Initializes the internal file table */
 int initializeFileTable (void);

 /* Prints all duplicate files logged in the file table by desc modified date */
 void printFileTable (void);

 /* Searches the file table for a particular file name. Then prints results */
 void findFile (const char *fileName);

 /* Returns the total number of files in the file table */
 long getFileCount (void);

 /* Free's the internal file table (and all files) */
 int freeFileTable (void);

#endif