#include "bros.h"
#include "brosKernel.h"
#include "brosGUI.h"
#include "brosFS.h"
#include "testApp1.h"
#include "testApp2.h"

#include "neslib.h"
#include <stdlib.h>

unsigned char* g_workRam = WRAM_START;

void main(void) {
    unsigned int testAppIdx;
    unsigned int frameCounter;

    #define TA1_LENGTH 28
    unsigned char TestApplication1[TA1_LENGTH] = {
        'B', 'R', 'O', 'S',
        0x01, 0x00, // ABI version
        0x00, 0x00, // No labels
        0x00, 0x00, // No extra ram
        0x10, 0x00, // Start at offset 16
        0x14, 0x00, // Proc at offset 20
        0x18, 0x00, // exit at offset 24
    };

    #define TA2_LENGTH 28
    unsigned char TestApplication2[TA2_LENGTH] = {
        'B', 'R', 'O', 'S',
        0x01, 0x00, // ABI version
        0x00, 0x00, // No labels
        0x00, 0x00, // No extra ram
        0x10, 0x00, // Start at offset 16
        0x14, 0x00, // Proc at offset 20
        0x18, 0x00, // exit at offset 24
    };

    // start() 1
    TestApplication1[16] = 0x20;
    TestApplication1[17] = (unsigned char)testApp1Start;
    TestApplication1[18] = (unsigned char)((unsigned int)testApp1Start >> 8);
    TestApplication1[19] = 0x60; // rts

    // proc() 1
    TestApplication1[20] = 0x20;
    TestApplication1[21] = (unsigned char)testApp1Proc;
    TestApplication1[22] = (unsigned char)((unsigned int)testApp1Proc >> 8);
    TestApplication1[23] = 0x60; // rts

    // atExit() 1
    TestApplication1[24] = 0x20;
    TestApplication1[25] = (unsigned char)(unsigned int)testApp1Exit;
    TestApplication1[26] = (unsigned char)((unsigned int)testApp1Exit >> 8);
    TestApplication1[27] = 0x60; // rts

    // start() 2
    TestApplication2[16] = 0x20;
    TestApplication2[17] = (unsigned char)testApp2Start;
    TestApplication2[18] = (unsigned char)((unsigned int)testApp2Start >> 8);
    TestApplication2[19] = 0x60; // rts

    // proc() 2
    TestApplication2[20] = 0x20;
    TestApplication2[21] = (unsigned char)testApp2Proc;
    TestApplication2[22] = (unsigned char)((unsigned int)testApp2Proc >> 8);
    TestApplication2[23] = 0x60; // rts

    // atExit() 2
    TestApplication2[24] = 0x20;
    TestApplication2[25] = (unsigned char)(unsigned int)testApp2Exit;
    TestApplication2[26] = (unsigned char)((unsigned int)testApp2Exit >> 8);
    TestApplication2[27] = 0x60; // rts

    initializeTable();

    initGui();

    initFS();

    setSystemError(SERR_SUCCESS);
    pal_col(1, 0x30); // set while color


    //allocateProcess((ApplicationHeader*)TestApplication1, TA1_LENGTH);
    //for (testAppIdx = 0; testAppIdx < TA1_LENGTH; ++testAppIdx) {
   //     unsigned int* target = getProcessTable()->processes[0].location + testAppIdx;
   //     *(target) = TestApplication1[testAppIdx];
   // }

    //getProcessTable()->processes[0].state = PS_LOADED;

    //allocateProcess((ApplicationHeader*)TestApplication2, TA2_LENGTH);
   // for (testAppIdx = 0; testAppIdx < TA2_LENGTH; ++testAppIdx) {
    //    unsigned int* target = getProcessTable()->processes[1].location + testAppIdx;
   //     *(target) = TestApplication2[testAppIdx];
    //}

    // getProcessTable()->processes[1].state = PS_LOADED;

    // rendering is disabled at the startup, the palette is all black


    // you can't put data into vram through vram_put while rendering is enabled
    // so you have to disable rendering to put things like text or a level map
    // into the nametable

    // there is a way to update small number of nametable tiles while rendering
    // is enabled, using set_vram_update and an update list

    put_str(NTADR_A(2, 2), "STARTED BROS");

    ppu_on_all(); // enable rendering

    frameCounter = 0;

    while(1) {
        ppu_wait_nmi();
        ++frameCounter;
        runApplications();

        if (frameCounter % 30 == 0) {
            processGUI();
        }
    }
}