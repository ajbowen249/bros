#ifndef BROS_API_H
#define BROS_API_H

#include "bros.h"
#include "brosFSTypes.h"
#include "brosGUITypes.h"

/**
 * To keep the API interface between apps and the kernel stable, we reserve a single place at the top of RAM for an
 * address to a table that itself has the addresses of accessible kernel functions. Sort of like manual dynamic
 * linking.
 *
 * This file is meant to be included by both the kernel and apps. It's the kernel's responsibility to initialize its
 * descriptor table and place its address in the correct spot.
 */

#define API_DESC_ADDR 0x0300

#define BROS_API_HEADER_VERSION 0x0001

typedef struct _APIDescriptor {
    unsigned int version;

    void (*exitCurrentProcess)();
    unsigned int (*getPid)();

    bool (*gui_addLabel)(GUIControl* label);
    void (*gui_guiNeedsRefresh)();

    EntryIndex (*fs_listFolder)(EntryIndex folderIndex, EntryIndex offset, EntryIndex maxEntries, FSEntry** entries);
    bool (*fs_openFile)(EntryIndex fileIndex, FSFileHandle* handle);
    bool (*fs_readBlock)(FSFileHandle* handle, unsigned char data[BFS_BLOCK_DATA_SIZE]);
    BlockAddress (*fs_getFileBlockSize)(EntryIndex fileIndex);
} APIDescriptor;

#endif
