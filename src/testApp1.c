#include "testApp1.h"
#include "brosGUI.h"
#include "brosKernel.h"
#include <stdlib.h>

int testApp1Counter;

GUIControl testApp1TitleLabel;
GUIControl testApp1CounterLabel;
GUIControl testApp1ExitLabel;

const char* testApp1Title = "TEST APP 1";
char testApp1CounterText[5] = "    ";
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
    if (testApp1Counter++ % 60 == 0) {
        testApp1CounterText[0] = ' ';
        testApp1CounterText[1] = ' ';
        testApp1CounterText[2] = ' ';
        testApp1CounterText[3] = ' ';
        itoa(testApp1Counter / 60, testApp1CounterText, 10);
        // TODO: No way this should be the right way...
        guiNeedsRefresh();
    }

    if (testApp1Counter > 60 * 10) {
        exitCurrentProcess();
    }
}

void testApp1Exit() {
    testApp1ExitLabel.isVisible = 1;
}
