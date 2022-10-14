#include "testApp2.h"
#include "brosKernel.h"
#include "brosGUI.h"
#include <stdlib.h>

int testApp2Counter;

GUIControl testApp2TitleLabel;
const char* testApp2Title = "TEST APP 2";

GUIControl testApp2CounterLabel;
char testApp2CounterText[5] = "    ";

GUIControl testApp2ExitLabel;
const char* testApp2ExitText = "TEST APP 2 EXIT";

void testApp2Start() {
    testApp2Counter = 10 * 60;
    addLabel(&testApp2TitleLabel);
    testApp2TitleLabel.control.label.maxLength = 10;
    testApp2TitleLabel.control.label.value = testApp2Title;
    testApp2TitleLabel.control.label.x = 2;
    testApp2TitleLabel.control.label.y = 10;

    addLabel(&testApp2CounterLabel);
    testApp2CounterLabel.control.label.maxLength = 4;
    testApp2CounterLabel.control.label.value = testApp2CounterText;
    testApp2CounterLabel.control.label.x = 2;
    testApp2CounterLabel.control.label.y = 11;

    addLabel(&testApp2ExitLabel);
    testApp2ExitLabel.control.label.maxLength = 15;
    testApp2ExitLabel.control.label.value = testApp2ExitText;
    testApp2ExitLabel.control.label.x = 2;
    testApp2ExitLabel.control.label.y = 12;
    testApp2ExitLabel.isVisible = 0;
}

void testApp2Proc() {
    if (testApp2Counter-- % 60 == 0) {
        testApp2CounterText[0] = ' ';
        testApp2CounterText[1] = ' ';
        testApp2CounterText[2] = ' ';
        testApp2CounterText[3] = ' ';
        itoa(testApp2Counter / 60, testApp2CounterText, 10);
        // TODO: No way this should be the right way...
        guiNeedsRefresh();
    }

    if (testApp2Counter <= 0) {
        exitCurrentProcess();
    }
}

void testApp2Exit() {
    testApp2ExitLabel.isVisible = 1;
}
