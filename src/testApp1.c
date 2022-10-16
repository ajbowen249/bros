#include "testApp1.h"
#include "brosGUI.h"
#include "brosKernel.h"
#include <stdlib.h>

#include "brosFS.h"

int testApp1Counter;

GUIControl testApp1TitleLabel;
GUIControl testApp1CounterLabel;
GUIControl testApp1ExitLabel;

const char* testApp1Title = "TEST APP 1";
char testApp1CounterText[10] = "         ";
const char* testApp1ExitText = "TEST APP 1 EXIT";

void testApp1Start() {
    testApp1Counter = 0;
    addLabel(&testApp1TitleLabel);
    testApp1TitleLabel.control.label.maxLength = 10;
    testApp1TitleLabel.control.label.value = testApp1Title;
    testApp1TitleLabel.control.label.x = 2;
    testApp1TitleLabel.control.label.y = 5;

    addLabel(&testApp1CounterLabel);
    testApp1CounterLabel.control.label.maxLength = 4;
    testApp1CounterLabel.control.label.value = testApp1CounterText;
    testApp1CounterLabel.control.label.x = 2;
    testApp1CounterLabel.control.label.y = 6;

    addLabel(&testApp1ExitLabel);
    testApp1ExitLabel.control.label.maxLength = 15;
    testApp1ExitLabel.control.label.value = testApp1ExitText;
    testApp1ExitLabel.control.label.x = 2;
    testApp1ExitLabel.control.label.y = 7;
    testApp1ExitLabel.isVisible = 0;
}

void testApp1Proc() {
    if (testApp1Counter > 0) {
        exitCurrentProcess();
    } else {
        itoa(BFS_MAX_BLOCKS, testApp1CounterText, 10);
        guiNeedsRefresh();
    }
}

void testApp1Exit() {
    testApp1ExitLabel.isVisible = 1;
}
