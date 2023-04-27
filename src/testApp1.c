#include "testApp1.h"
#include "brosAPI.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static GUIControl testApp1TitleLabel;
static GUIControl ta1Label1;

static const char* testApp1Title = "FILE READ TEST";

static char ta1Label1Text[18] = "                 ";

#define NUM_LINES 10
GUIControl fileLines[NUM_LINES];
char fileText[NUM_LINES][13];

static BlockAddress fileSize;
static bool gotFile;
static FSFileHandle handle;
APIDescriptor* api;

static unsigned char block[BFS_BLOCK_DATA_SIZE];

void testApp1Start() {
    EntryIndex fileIndex = 6;
    char lineIndex = 0;
    APIDescriptor** apiPtr = API_DESC_ADDR;
    api = *apiPtr;

    api->gui_addLabel(&testApp1TitleLabel);
    testApp1TitleLabel.control.label.maxLength = 14;
    testApp1TitleLabel.control.label.value = testApp1Title;
    testApp1TitleLabel.control.label.x = 2;
    testApp1TitleLabel.control.label.y = 5;

    api->gui_addLabel(&ta1Label1);
    ta1Label1.control.label.maxLength = 17;
    ta1Label1.control.label.value = ta1Label1Text;
    ta1Label1.control.label.x = 2;
    ta1Label1.control.label.y = 6;

    fileSize = api->fs_getFileBlockSize(fileIndex);
    gotFile = fileSize != BLOCK_ADDRESS_NONE;
    if (!gotFile) {
        sprintf(ta1Label1Text, "NO FILE");
        api->exitCurrentProcess();
        api->gui_guiNeedsRefresh();
        return;
    } else {
        char* name;
        size_t nameLength;
        size_t extLength;
        api->fs_openFile(fileIndex, &handle);
        name = handle.entry->name.name;
        nameLength = strnlen(name, BFS_NAME_LENGTH);
        extLength = strnlen(handle.entry->name.extension, BFS_NAME_LENGTH);
        strcpy(ta1Label1Text, "TEST             ");
        strncpy(ta1Label1Text, name, BFS_NAME_LENGTH);
        if (extLength > 0) {
            strncpy(ta1Label1Text + nameLength, BFS_EXT_SEPARATOR, 1);
            strncpy(ta1Label1Text + nameLength + 1, handle.entry->name.extension, BFS_EXTENSION_LENGTH);
        }
    }

    for (lineIndex = 0; lineIndex < NUM_LINES && lineIndex < fileSize; lineIndex++) {
        api->gui_addLabel(&fileLines[lineIndex]);
        fileLines[lineIndex].control.label.value = fileText[lineIndex];
        fileLines[lineIndex].control.label.x = 2;
        fileLines[lineIndex].control.label.y = 8 + lineIndex;

        api->fs_readBlock(&handle, block);
        strncpy(fileText[lineIndex], block, BFS_BLOCK_DATA_SIZE);
    }

    api->gui_guiNeedsRefresh();
    api->exitCurrentProcess();
}

void testApp1Proc() {

}

void testApp1Exit() {
}
