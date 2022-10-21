#include "testApp1.h"
#include "brosGUI.h"
#include "brosKernel.h"
#include "brosFS.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static GUIControl testApp1TitleLabel;
static GUIControl ta1Label1;
static GUIControl ta1Label2;
static GUIControl testApp1ExitLabel;

static const char* testApp1Title = "FILE SIZE TEST";

static char ta1Label1Text[10] = "         ";
static char ta1Label2Text[(BFS_BLOCK_DATA_SIZE) + 1] = "123456789ABC";

static const char* testApp1ExitText = "TEST APP 1 EXIT";

static int blockCounter;

static BlockAddress fileSize;
static bool gotFile;
static FSFileHandle handle;

static unsigned int counter;
static unsigned char block[BFS_BLOCK_DATA_SIZE];

void testApp1Start() {
    EntryIndex fileIndex = 6;
    counter = 0;

    addLabel(&testApp1TitleLabel);
    testApp1TitleLabel.control.label.maxLength = 14;
    testApp1TitleLabel.control.label.value = testApp1Title;
    testApp1TitleLabel.control.label.x = 2;
    testApp1TitleLabel.control.label.y = 5;

    addLabel(&ta1Label1);
    ta1Label1.control.label.maxLength = 4;
    ta1Label1.control.label.value = ta1Label1Text;
    ta1Label1.control.label.x = 2;
    ta1Label1.control.label.y = 6;

    addLabel(&ta1Label2);
    ta1Label2.control.label.maxLength = 13;
    ta1Label2.control.label.value = ta1Label2Text;
    ta1Label2.control.label.x = 2;
    ta1Label2.control.label.y = 7;

    addLabel(&testApp1ExitLabel);
    testApp1ExitLabel.control.label.maxLength = 15;
    testApp1ExitLabel.control.label.value = testApp1ExitText;
    testApp1ExitLabel.control.label.x = 2;
    testApp1ExitLabel.control.label.y = 8;
    testApp1ExitLabel.isVisible = 0;

    fileSize = getFileBlockSize(fileIndex);
    gotFile = fileSize != BLOCK_ADDRESS_NONE;
    if (!gotFile) {
        sprintf(ta1Label1Text, "NO FILE");
    } else {
        sprintf(ta1Label1Text, "SIZE %i", fileSize);
    }

    openFile(fileIndex, &handle);
    blockCounter = 0;
    guiNeedsRefresh();
}

void testApp1Proc() {
    if (counter++ % 60 != 0) {
        return;
    }

    if (!gotFile) {
        return;
    }

    if (blockCounter >= fileSize) {
        handle.blockAddress = handle.entry->entry.file.firstBlock;
        sprintf(ta1Label2Text, "%s", "PAST END");
        guiNeedsRefresh();
        exitCurrentProcess();
        return;
    }
    sprintf(ta1Label2Text, "%i", handle.blockAddress);
    blockCounter++;
    guiNeedsRefresh();

    if (readBlock(&handle, block)) {
    //    sprintf(ta1Label2Text, "%s", "            ");
      //  strncpy(ta1Label2Text, block, BFS_BLOCK_DATA_SIZE);
    } else {
        sprintf(ta1Label2Text, "%s", "NO FILE");
    }

    guiNeedsRefresh();
}

void testApp1Exit() {
}
