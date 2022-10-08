#include "bros.h"
#include "brosGUI.h"

#include "neslib.h"
#include <stdlib.h>

// The actual process table
ProcessTable g_processTable;

// The ID of the process currently being called
unsigned int g_pid;

// The last system error
SystemError g_systemError;


// TODO: Redo build system. This file is HUGE!!!!

#pragma region Kernel Routines

// TODO: These eventually need to be declared in a way where we know how to call them from apps

void __fastcall__ setSystemError(SystemError err);
void setSystemError(SystemError err) {
    g_systemError = err;
}

// put a string into the nametable
void __fastcall__ put_str(unsigned int adr, const char* str);
void put_str(unsigned int adr, const char* str) {
    vram_adr(adr);

    while (1) {
        if (!*str) {
            break;
        }

        vram_put((*str++) - 0x20); // -0x20 because ASCII code 0x20 is placed in tile 0 of the CHR
    }
}

void exitCurrentProcess() {
    if (g_pid == BROS_INVALID_PID || g_pid > MAX_BROS_PROCESSES) {
        setSystemError(SERR_INVALID_PID);
    } else {
        g_processTable.processes[g_pid].state = PS_AWAITING_EXIT;
    }
}

#pragma endregion

#pragma region GUI server implementations

// Yeah, yeah, fix the build system and get these out of here...

#define MAX_GUI_CONTROLS 50

GUIControl* g_guiControls[MAX_GUI_CONTROLS];

GUIError g_guiError;

bool g_guiNeedsRefresh;

void initGui() {
    int i;
    
    for (i = 0; i < MAX_GUI_CONTROLS; ++i) {
        g_guiControls[i] = nullptr;
    }  

    g_guiNeedsRefresh = 0;

    g_guiError = GE_SUCCESS;
}

bool addLabel(GUIControl* label) {
    unsigned int controlIndex;

    if (g_pid == BROS_INVALID_PID) {
        g_guiError = GE_NOT_CALLED_FROM_APP;
        return 0;
    }

    for (controlIndex = 0; controlIndex < MAX_GUI_CONTROLS; ++controlIndex) {
        if (!g_guiControls[controlIndex]) {
            break;
        }
    }

    if (controlIndex >= MAX_GUI_CONTROLS) {
        g_guiError = GE_OUT_OF_MEMORY;
        return 0;
    }

    label->label.pid = g_pid;
    label->label.type = GCT_LABEL;
    label->label.maxLength = 0;
    label->label.value = nullptr;

    g_guiNeedsRefresh  = 1;

    return 1;
}

void invalidate() {
    g_guiNeedsRefresh = 1;
}

#pragma endregion

#pragma region GUI server "internals"

void clearScreen() {
    // IMPROVE: This is SLOW and dumb. Attempts at just writing the block have failed.
    //          Must be a better way to do this.

    unsigned int y;

    for (y = 0; y < NAMETABLE_HEIGHT; ++y) {
        put_str(NTADR_A(0, y), "                                ");
    }
}

void drawLabel(GUIControl* label) {/*
    unsigned char length = label->label.maxLength;
    char* source = label->label.value;
    unsigned int x = label->label.x;
    unsigned int y = label->label.y;
    unsigned char maxX = x + label->label.maxLength;
    bool pastEnd = 0;
    unsigned int index;

    if (y > NAMETABLE_MAX_Y) {
        return;
    }

    if (maxX > NAMETABLE_MAX_X) {
        maxX = NAMETABLE_MAX_X;
    }

    if (x > maxX) {
        return;
    }
*/
    put_str(NTADR_A(label->label.x, label->label.y), label->label.value);
    // put_str(NTADR_A(2, 5), "TEST");
    return;/*

    vram_adr(NTADR_A(x, y));

    for (index = 0; index < length; ++index) {
        char ch;
        if (!pastEnd) {
            ch = source[index];
        } else {
            ch = " ";
        }

        if (!ch) {
            pastEnd = 1;
            ch = " ";
        }

        vram_put(ch - 0x20);

        ++x;
        if (x > maxX) {
            return;
        }
    }*/
}

void processGUI() {
    unsigned int controlIndex;
    GUIControl* control;


    if (!g_guiNeedsRefresh) {
        return;
    }

    ppu_off();
    clearScreen();

    put_str(NTADR_A(2, 5), "TEST");
    ppu_on_all();
    g_guiNeedsRefresh = 0;
    return;

    for (controlIndex = 0; controlIndex < MAX_GUI_CONTROLS; ++controlIndex) {
        control = g_guiControls[controlIndex];
        if (control) {
            switch (control->nothing.type)
            {
            case GCT_LABEL:
                drawLabel(control);
                break;
            default:
                break;
            }
        }
    }

    g_guiNeedsRefresh = 0;

    ppu_on_all();
}

#pragma endregion

bool allocateProcess(const ApplicationHeader* header, unsigned int fileSize) {
    unsigned int offset = 0;
    unsigned int pid = 0;
    unsigned int memorySize;
    Process* process;

    if (header->b != 'B' || header->r != 'R' || header->o != 'O' || header->s != 'S') {
        setSystemError(SERR_INVALID_APPLICATION_HEADER);
        return 0;
    }

    memorySize = fileSize + header->ramSize;

    if (memorySize > BROS_APPLICATION_RAM_SIZE) {
        setSystemError(SERR_NOT_ENOUGH_MEMORY);
        return 0;
    }

    for (; pid < MAX_BROS_PROCESSES; ++pid) {
        unsigned int endAddress = BROS_APPLICATION_RAM_SIZE;
        process = &g_processTable.processes[pid];
        if (process->state != PS_INACTIVE) {
            offset += process->size;
            continue;
        }

        // We found an open PID. Is there enough memory here?
        if (pid < MAX_BROS_PROCESSES - 1) {
            Process* nextProcess;
            unsigned int maybeNextPid = pid + 1;
            for (; maybeNextPid < MAX_BROS_PROCESSES; ++maybeNextPid) {
                nextProcess = &g_processTable.processes[maybeNextPid];
                if (nextProcess->state != PS_INACTIVE) {
                    endAddress = nextProcess->location;
                }
            }
        }

        if ((endAddress - offset) < memorySize) {
            continue;
        }

        process->location = offset + WRAM_START;
        process->size = memorySize;

        process->start = header->startOffset ? WRAM_START + offset + header->startOffset : 0;
        process->proc = header->procOffset ? WRAM_START + offset + header->procOffset : 0;
        process->onExit = header->exitOffset ? WRAM_START + offset + header->exitOffset : 0;
        process->state = PS_LOADING;

        return 1;
    }

    setSystemError(SERR_NOT_ENOUGH_MEMORY);
    return 0;
}

void runApplications() {
    static Process* process;

    for (g_pid = 0; g_pid < MAX_BROS_PROCESSES; ++g_pid) {
        process = &g_processTable.processes[g_pid];

        switch (process->state)
        {
        case PS_LOADED:
            if (process->start) {
               process->start();
            }
            process->state = PS_RUNNING;
            break;
        case PS_RUNNING:
            if (process->proc) {
                process->proc();
            } else {
                // Flag exit if no proc
                process->state = PS_AWAITING_EXIT;
            }
            break;
        case PS_AWAITING_EXIT:
            if (process->onExit) {
                process->onExit();
            }
            process->state = PS_INACTIVE;
        default:
            break;
        }
    }

    g_pid = BROS_INVALID_PID;
}

void initializeTable() {
    unsigned int pid;
    Process* process;
    for (pid = 0; pid < MAX_BROS_PROCESSES; ++pid) {
        process = &g_processTable.processes[pid];
        process->state = PS_INACTIVE;
    }
}

// int testApp1Counter;

GUIControl testApp1TitleLabel;
const char* testApp1Title = "TEST APP 1";

void testApp1Start() {
    // testApp1Counter = 0;
    // ppu_off();
    // put_str(NTADR_A(2, 5), "TEST APP 1 START");
    // ppu_on_all();
    addLabel(&testApp1TitleLabel);
    testApp1TitleLabel.label.maxLength = 10;
    testApp1TitleLabel.label.value = testApp1Title;
    testApp1TitleLabel.label.x = 2;
    testApp1TitleLabel.label.y = 5;
}

void testApp1Proc() {
    // char outputLine[20];
    // int i;
    // for (i = 0; i < 20; --i) {
    //     outputLine[i] = 0;
    // }

    // if (testApp1Counter++ % 60 == 0) {
    //     ppu_off();
    //     itoa(testApp1Counter / 60, outputLine, 10);
    //     put_str(NTADR_A(2, 6), "    ");
    //     put_str(NTADR_A(2, 6), outputLine);
    //     ppu_on_all();
    // }

    // if (testApp1Counter > 60 * 10) {
    //     exitCurrentProcess();
    // }
}

void testApp1Exit() {
    // ppu_off();
    // put_str(NTADR_A(2, 7), "TEST APP 1 EXIT");
    // ppu_on_all();
}

int testApp2Counter;

void testApp2Start() {
    testApp2Counter = 60 * 10;
    ppu_off();
    put_str(NTADR_A(2, 9), "TEST APP 2 START");
    ppu_on_all();
}

void testApp2Proc() {
    char outputLine[20];
    int i;

    for (i = 0; i < 20; ++i) {
        outputLine[i] = 0;
    }

    if (testApp2Counter-- % 60 == 0) {
        ppu_off();
        itoa(testApp2Counter / 60, outputLine, 10);
        put_str(NTADR_A(2, 10), "    ");
        put_str(NTADR_A(2, 10), outputLine);
        ppu_on_all();
    }

    if (testApp2Counter == 0) {
        exitCurrentProcess();
    }
}

void testApp2Exit() {
    ppu_off();
    put_str(NTADR_A(2, 11), "TEST APP 2 EXIT");
    ppu_on_all();
}


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

    g_pid = BROS_INVALID_PID;
    setSystemError(SERR_SUCCESS);
    pal_col(1, 0x30); // set while color


    allocateProcess((ApplicationHeader*)TestApplication1, TA1_LENGTH);
    for (testAppIdx = 0; testAppIdx < TA1_LENGTH; ++testAppIdx) {
        unsigned int* target = g_processTable.processes[0].location + testAppIdx;
        *(target) = TestApplication1[testAppIdx];
    }

    g_processTable.processes[0].state = PS_LOADED;

    // allocateProcess((ApplicationHeader*)TestApplication2, TA2_LENGTH);
    // for (testAppIdx = 0; testAppIdx < TA2_LENGTH; ++testAppIdx) {
        // unsigned int* target = g_processTable.processes[1].location + testAppIdx;
        // *(target) = TestApplication2[testAppIdx];
    // }

   // g_processTable.processes[1].state = PS_LOADED;

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