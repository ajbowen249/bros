#ifndef BROS_FS_H
#define BROS_FS_H

#include "brosFSTypes.h"

void initFS();

// Not really going to build a concept of locks for now. Similar to the "no security" theme; just behave

// Also going to skip a concept of string paths for the first round.
// Basing core functions on EntryIndex for now, then something in the future can resolve EntryIndices from paths.

/**
 * Populates a presized list of entries contained in the given folder
 *
 * @param folderIndex The location of the folder entry in the table
 * @param offset The number of entries in the folder to skip before populating the table
 * @param maxEntries The number of entries that will fit in `entries`
 * @param entries An array that stores `maxEntries` entries
 */
EntryIndex listFolder(EntryIndex folderIndex, EntryIndex offset, EntryIndex maxEntries, FSEntry** entries);

/**
 * Initializes the given file handle if a file entry exists at the given entry index.
 * Returns false if the entry wasn't valid.
 */
bool openFile(EntryIndex fileIndex, FSFileHandle* handle);

/**
 * Copies a block of data to the given buffer and advances the handle's block pointer.
 * Returns false if no data was read
 */
bool readBlock(FSFileHandle* handle, unsigned char data[BFS_BLOCK_DATA_SIZE]);

/**
 * Iterates the blockfield to count file block size
 * Returns `BLOCK_ADDRESS_NONE` for bad file index
 */
BlockAddress getFileBlockSize(EntryIndex fileIndex);

#endif
