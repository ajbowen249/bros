#include "testApp1.h"
#include "brosGUI.h"
#include "brosKernel.h"
#include <stdlib.h>

#include "brosFS.h"

int testApp1Counter;

GUIControl testApp1TitleLabel;
GUIControl ta1Label1;
GUIControl ta1Label2;
GUIControl testApp1ExitLabel;

const char* testApp1Title = "TEST APP 1";

char ta1Label1Text[10] = "         ";
char ta1Label2Text[10] = "         ";

const char* testApp1ExitText = "TEST APP 1 EXIT";

void testApp1Start() {
    testApp1Counter = 0;
    addLabel(&testApp1TitleLabel);
    testApp1TitleLabel.control.label.maxLength = 10;
    testApp1TitleLabel.control.label.value = testApp1Title;
    testApp1TitleLabel.control.label.x = 2;
    testApp1TitleLabel.control.label.y = 5;

    addLabel(&ta1Label1);
    ta1Label1.control.label.maxLength = 4;
    ta1Label1.control.label.value = ta1Label1Text;
    ta1Label1.control.label.x = 2;
    ta1Label1.control.label.y = 6;

    addLabel(&ta1Label2);
    ta1Label2.control.label.maxLength = 4;
    ta1Label2.control.label.value = ta1Label2Text;
    ta1Label2.control.label.x = 2;
    ta1Label2.control.label.y = 7;

    addLabel(&testApp1ExitLabel);
    testApp1ExitLabel.control.label.maxLength = 15;
    testApp1ExitLabel.control.label.value = testApp1ExitText;
    testApp1ExitLabel.control.label.x = 2;
    testApp1ExitLabel.control.label.y = 8;
    testApp1ExitLabel.isVisible = 0;
}

void testApp1Proc() {
    if (testApp1Counter > 0) {
        exitCurrentProcess();
    } else {
        unsigned int test = 0xabcd;
        unsigned char* testB = &test;

        itoa(testB[0], ta1Label1Text, 10);
        itoa(testB[1], ta1Label2Text, 10);
        guiNeedsRefresh();
    }
}

void testApp1Exit() {
    testApp1ExitLabel.isVisible = 1;
}
