#ifndef BROS_FS_TYPES_H
#define BROS_FS_TYPES_H

#include "bros.h"

#define BFS_VERSION 1

/**
 * IMPROVE: Build a better filesystem. Physical version could offload most of this to a microcontroller with an SD card
 *          library.
 *
 * BrOS FS Version 1
 *
 * For simplicity reasons, this version of the filesystem involves lots of fixed allocation. This is mostly here as
 * placeholder to get started.
 * 
*/

#define BFS_DEFAULT_DISK_VOLUME_SIZE 0x1000

/**
 * The beginning of a BROS filesystem
*/
typedef struct _FSHeader {
    // The literal B character
    char b;
    // The literal F character
    char f;
    // The literal S character
    char s;
    // The bfs version
    unsigned char version;

    // The total size of the volume
    unsigned long volumeSize;
} FSHeader;

#define BFS_HEADER_SIZE sizeof(FSHeader)

#define BFS_HEADER_START WRAM_START
#define BFS_TABLE_START BFS_HEADER_START + BFS_HEADER_SIZE

typedef enum _FSEntryType {
    FSE_NONE = 0,
    FSE_FOLDER = 1,
    FSE_FILE = 2,
} FSEntryType;

#define BFS_NAME_LENGTH 12
#define BFS_EXTENSION_LENGTH 4

#define BFS_BLOCK_DATA_SIZE 12

typedef struct _FSName {
    char name[BFS_NAME_LENGTH];
    char extension[BFS_EXTENSION_LENGTH];
} FSName;

typedef unsigned int BlockAddress;
#define BLOCK_ADDRESS_NONE 0xffff

typedef struct _FSFileEntry {
    BlockAddress firstBlock;
} FSFileEntry;

typedef struct _FSFolderEntry {
    char placeholder;
} FSFolderEntry;

#define BFS_ROOT_FOLDER 255

typedef unsigned char EntryIndex;

typedef struct _FSEntry {
    FSEntryType type;
    EntryIndex folderIndex;
    FSName name;
    union {
        FSFileEntry file;
        FSFolderEntry folder;
    } entry;
} FSEntry;

#define BFS_ENTRY_SIZE sizeof(FSEntry)
#define BFS_MAX_ENTRIES 50

typedef struct _FSTable {
    FSEntry entries[BFS_MAX_ENTRIES];
} FSTable;

#define BFS_BLOCKFIELD_START BFS_TABLE_START + sizeof(FSTable)

typedef struct _FSBlock {
    unsigned char isOccupied;
    unsigned char reserved;
    BlockAddress nextBlock;
    unsigned char data[BFS_BLOCK_DATA_SIZE];
} FSBlock;

#define BFS_BLOCK_RECORD_SIZE sizeof(FSBlock)

#define BFS_AVAILABLE_SPACE (BFS_DEFAULT_DISK_VOLUME_SIZE - BFS_HEADER_SIZE - (BFS_MAX_ENTRIES * BFS_ENTRY_SIZE))
#define BFS_MAX_BLOCKS (BFS_AVAILABLE_SPACE / BFS_BLOCK_RECORD_SIZE)

#define BFS_EXT_SEPARATOR "."

typedef struct _FSBlockField {
    FSBlock blocks[BFS_MAX_BLOCKS];
} FSBlockField;

/**
 * A handle to a file on disk
*/
typedef struct _FSFileHandle {
    // The file description
    FSEntry* entry;

    /**
     * The block in the volume.
     * `BLOCK_ADDRESS_NONE` is set when advanced past end of file
     */
    BlockAddress blockAddress;
} FSFileHandle;

#endif
