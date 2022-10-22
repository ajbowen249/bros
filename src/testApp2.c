#include "testApp2.h"
#include "brosKernel.h"
#include "brosGUI.h"
#include "brosFS.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

GUIControl testApp2TitleLabel;
const char* testApp2Title = "FOLDER LIST TEST";

GUIControl folderLabels[10];
char folderLabelText[10][17];

void testApp2Start() {
    unsigned char foundEntries;
    unsigned int i;
    FSEntry* entries[10];

    addLabel(&testApp2TitleLabel);
    testApp2TitleLabel.control.label.maxLength = 10;
    testApp2TitleLabel.control.label.value = testApp2Title;
    testApp2TitleLabel.control.label.x = 2;
    testApp2TitleLabel.control.label.y = 2;

    foundEntries = listFolder(BFS_ROOT_FOLDER, 0, 10, entries);

    if (foundEntries == 0) {
        addLabel(&folderLabels[0]);
        strcpy(folderLabelText[0], "NONE");
        folderLabels[0].control.label.maxLength = 4;
        folderLabels[0].control.label.value = folderLabelText[0];
        folderLabels[0].control.label.x = 2;
        folderLabels[0].control.label.y = 3;
    } else {
        for (i = 0; i < foundEntries; ++i) {
            FSEntry* entry = entries[i];
            char* name = entry->name.name;
            size_t nameLength = strnlen(name, BFS_NAME_LENGTH);
            size_t extLength = strnlen(entry->name.extension, BFS_NAME_LENGTH);

            addLabel(&folderLabels[i]);
            strcpy(folderLabelText[i], "                ");
            strncpy(folderLabelText[i], name, BFS_NAME_LENGTH);
            if (extLength > 0) {
                strncpy(folderLabelText[i] + nameLength, BFS_EXT_SEPARATOR, 1);
                strncpy(folderLabelText[i] + nameLength + 1, entry->name.extension, BFS_EXTENSION_LENGTH);
            }

            folderLabels[i].control.label.maxLength = 17;
            folderLabels[i].control.label.value = folderLabelText[i];
            folderLabels[i].control.label.x = 2;
            folderLabels[i].control.label.y = 4 + i;
        }
    }

    guiNeedsRefresh();
    exitCurrentProcess();
}

void testApp2Proc() {
}

void testApp2Exit() {
}
