#include "brosFS.h"
#include "bros.h"

static FSHeader* fsHeader = BFS_HEADER_START;
static FSTable* fsTable = BFS_TABLE_START;
static FSBlockField* fsBlockField = BFS_BLOCKFIELD_START;

bool fsIsValid() {
    if (
        fsHeader->b != 'B' ||
        fsHeader->f != 'F' ||
        fsHeader->s != 'S'
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
