#include "brosFS.h"
#include "bros.h"
#include "brosKernel.h"

#include <string.h>

static FSHeader* fsHeader = BFS_HEADER_START;
static FSTable* fsTable = BFS_TABLE_START;
static FSBlockField* fsBlockField = BFS_BLOCKFIELD_START;

bool fsIsValid() {
    if (
        fsHeader->b != 'B' ||
        fsHeader->f != 'F' ||
        fsHeader->s != 'S' ||
        fsHeader->version != BFS_VERSION
    ) {
        return 0;
    }

    return 1;
}

void formatFS() {
    int i;

    fsHeader->b = "B";
    fsHeader->f = "F";
    fsHeader->s = "S";
    fsHeader->version = BFS_VERSION;

    fsHeader->volumeSize = BFS_DEFAULT_DISK_VOLUME_SIZE;

    for (i = 0; i < BFS_MAX_ENTRIES; ++i) {
        fsTable->entries[i].type = FSE_NONE;
    }

    for (i = 0; i < BFS_MAX_BLOCKS; ++i) {
        fsBlockField->blocks[i].isOccupied = 0;
    }
}

void initFS() {
    if (fsIsValid()) {
        return;
    }

    formatFS();
}

EntryIndex listFolder(EntryIndex folderIndex, EntryIndex offset, EntryIndex maxEntries, FSEntry** entries) {
    EntryIndex foundItems;
    unsigned int i;
    FSEntryType entryType = fsTable->entries[folderIndex].type;
    if (entryType == FSE_NONE) {
        setSystemError(SERR_FS_INVALID_INDEX);
        return 0;
    }

    if (entryType != FSE_FOLDER) {
        setSystemError(SERR_FS_ENTRY_NOT_FOLDER);
        return 0;
    }

    for (i = offset; i < BFS_MAX_ENTRIES; ++i) {
        FSEntry* entry = &fsTable->entries[i];
        if (entry->folderIndex != folderIndex) {
            continue;
        }

        if (entry->type != FSE_NONE) {
            entries[foundItems] = entry;
            ++foundItems;
        }

        if (foundItems >= maxEntries) {
            break;
        }
    }

    return foundItems;
}

bool openFile(EntryIndex fileIndex, FSFileHandle* handle) {
    FSEntry* entry = &(fsTable->entries[fileIndex]);
    if (entry->type == FSE_NONE) {
        setSystemError(SERR_FS_INVALID_INDEX);
        return 0;
    }

    if (entry->type != FSE_FILE) {
        setSystemError(SERR_FS_ENTRY_NOT_FILE);
        return 0;
    }

    handle->entry = entry;
    handle->blockAddress = entry->entry.file.firstBlock;

    return 1;
}

bool readBlock(FSFileHandle* handle, unsigned char data[BFS_BLOCK_DATA_SIZE]) {
    FSBlock* block;
    if (handle->blockAddress == BLOCK_ADDRESS_NONE) {
        setSystemError(SERR_FS_BLOCK_INVALID);
        return 0;
    }

    block = &(fsBlockField->blocks[handle->blockAddress]);
    if (!block->isOccupied) {
        setSystemError(SERR_FS_BLOCK_INVALID);
        return 0;
    }

    memcpy(data, block->data, BFS_BLOCK_DATA_SIZE);

    handle->blockAddress = block->nextBlock;

    return 1;
}

BlockAddress getFileBlockSize(EntryIndex fileIndex) {
    FSFileHandle handle;
    BlockAddress count = 0;
    FSBlock* block;

    if (!openFile(fileIndex, &handle)) {
        return BLOCK_ADDRESS_NONE;
    }

    if (handle.blockAddress == BLOCK_ADDRESS_NONE) {
        return 0;
    }

    while (handle.blockAddress != BLOCK_ADDRESS_NONE) {
        block = &(fsBlockField->blocks[handle.blockAddress]);
        if (!block->isOccupied) {
            setSystemError(SERR_FS_BLOCK_INVALID);
            return BLOCK_ADDRESS_NONE;
        }

        ++count;
        handle.blockAddress = block->nextBlock;
    }

    return count;
}
