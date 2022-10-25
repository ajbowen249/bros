#include "bros.h"
#include "brosKernel.h"
#include "brosGUI.h"

#include <string.h>
#include <stdio.h>

#pragma code-name(push, "APP_DATA")
#pragma bss-name(push, "APP_BSS")
#pragma data-name(push, "APP_DATA")
#pragma rodata-name(push, "APP_DATA")

GUIControl startLabel;
GUIControl procLabel;
GUIControl endLabel;

const char* startText = "TEST APP 3 START";
const char* procText = "          ";
const char* endText = "TEST APP 3 END";

int counter;

void start() {
    counter = 5;
    addLabel(&startLabel);
    startLabel.control.label.x = 2;
    startLabel.control.label.y = 2;
    startLabel.control.label.value = startText;
    startLabel.control.label.maxLength = 16;

    addLabel(&procLabel);
    procLabel.control.label.x = 2;
    procLabel.control.label.y = 3;
    procLabel.control.label.value = procText;
    procLabel.control.label.maxLength = 16;
    procLabel.isVisible = 1;

    addLabel(&endLabel);
    endLabel.control.label.x = 2;
    endLabel.control.label.y = 4;
    endLabel.control.label.value = endText;
    endLabel.control.label.maxLength = 16;
    endLabel.isVisible = 0;

    guiNeedsRefresh();
}

void proc() {
    if (counter++ % 30 == 0) {
        sprintf(procText, "PROC %i", counter);
        guiNeedsRefresh();
    }

    if (counter > 40) {
        exitCurrentProcess();
    }
}

void atExit() {
    endLabel.isVisible = 1;
    guiNeedsRefresh();
}

#pragma rodata-name(pop)
#pragma data-name(pop)
#pragma bss-name(pop)
#pragma code-name(pop)
