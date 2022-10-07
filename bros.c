#include "bros.h"

#include "neslib.h"
#include <stdlib.h>

// The actual process table
ProcessTable g_processTable;

// The last system error
SystemError g_systemError;

// The ID of the process currently being called
unsigned int g_pid;

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
    static unsigned int pid;
    static Process* process;

    for (pid = 0; pid < MAX_BROS_PROCESSES; ++pid) {
        process = &g_processTable.processes[pid];

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
}

void initializeTable() {
    unsigned int pid;
    Process* process;
    for (pid = 0; pid < MAX_BROS_PROCESSES; ++pid) {
        process = &g_processTable.processes[pid];
        process->state = PS_INACTIVE;
    }
}

int testApp1Counter;

void testApp1Start() {
    testApp1Counter = 0;
    ppu_off();
    put_str(NTADR_A(2, 5), "TEST APP 1 START");
    ppu_on_all();
    return;
}

void testApp1Proc() {
    char outputLine[20];
    int i;
    for (i = 0; i < 20; --i) {
        outputLine[i] = 0;
    }
    if (testApp1Counter++ % 60 == 0) {
        ppu_off();
        itoa(testApp1Counter / 60, outputLine, 10);
        put_str(NTADR_A(2, 6), "    ");
        put_str(NTADR_A(2, 6), outputLine);
        ppu_on_all();
    }
    return;
}

int testApp2Counter;

void testApp2Start() {
    testApp2Counter = 1000 * 30;
    ppu_off();
    put_str(NTADR_A(2, 8), "TEST APP 2 START");
    ppu_on_all();
    return;
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
        put_str(NTADR_A(2, 9), "    ");
        put_str(NTADR_A(2, 9), outputLine);
        ppu_on_all();
    }
    return;
}


unsigned char* g_workRam = WRAM_START;

void main(void) {
    unsigned int testAppIdx;

    unsigned int testApp1StartAddr = (unsigned int)testApp1Start;
    unsigned int testApp1ProcAddr = (unsigned int)testApp1Proc;

    unsigned int testApp2StartAddr = (unsigned int)testApp2Start;
    unsigned int testApp2ProcAddr = (unsigned int)testApp2Proc;

    #define TA1_LENGTH 24
    unsigned char TestApplication1[TA1_LENGTH] = {
        'B', 'R', 'O', 'S',
        0x01, 0x00, // ABI version
        0x00, 0x00, // No labels
        0x00, 0x00, // No extra ram
        0x10, 0x00, // Start at offset 16
        0x14, 0x00, // Proc at offset 20
        0x00, 0x00, // No exit hook
    };

    #define TA2_LENGTH 24
    unsigned char TestApplication2[TA2_LENGTH] = {
        'B', 'R', 'O', 'S',
        0x01, 0x00, // ABI version
        0x00, 0x00, // No labels
        0x00, 0x00, // No extra ram
        0x10, 0x00, // Start at offset 16
        0x14, 0x00, // Proc at offset 20
        0x00, 0x00, // No exit hook
    };

    // start()
    TestApplication1[16] = 0x20;
    TestApplication1[17] = (unsigned char)testApp1StartAddr;
    TestApplication1[18] = (unsigned char)(testApp1StartAddr >> 8);
    TestApplication1[19] = 0x60; // rts

    // proc()
    TestApplication1[20] = 0x20;
    TestApplication1[21] = (unsigned char)testApp1ProcAddr;
    TestApplication1[22] = (unsigned char)(testApp1ProcAddr >> 8);
    TestApplication1[23] = 0x60; // rts

    // start()
    TestApplication2[16] = 0x20;
    TestApplication2[17] = (unsigned char)testApp2StartAddr;
    TestApplication2[18] = (unsigned char)(testApp2StartAddr >> 8);
    TestApplication2[19] = 0x60; // rts

    // proc()
    TestApplication2[20] = 0x20;
    TestApplication2[21] = (unsigned char)testApp2ProcAddr;
    TestApplication2[22] = (unsigned char)(testApp2ProcAddr >> 8);
    TestApplication2[23] = 0x60; // rts

    initializeTable();

    g_pid = BROS_INVALID_PID;
    setSystemError(SERR_SUCCESS);
    pal_col(1, 0x30); // set while color


    allocateProcess((ApplicationHeader*)TestApplication1, TA1_LENGTH);
    for (testAppIdx = 0; testAppIdx < TA1_LENGTH; ++testAppIdx) {
        unsigned int* target = g_processTable.processes[0].location + testAppIdx;
        *(target) = TestApplication1[testAppIdx];
    }

    g_processTable.processes[0].state = PS_LOADED;

    allocateProcess((ApplicationHeader*)TestApplication2, TA2_LENGTH);
    for (testAppIdx = 0; testAppIdx < TA2_LENGTH; ++testAppIdx) {
        unsigned int* target = g_processTable.processes[1].location + testAppIdx;
        *(target) = TestApplication2[testAppIdx];
    }

   g_processTable.processes[1].state = PS_LOADED;

    // rendering is disabled at the startup, the palette is all black


    // you can't put data into vram through vram_put while rendering is enabled
    // so you have to disable rendering to put things like text or a level map
    // into the nametable

    // there is a way to update small number of nametable tiles while rendering
    // is enabled, using set_vram_update and an update list

    put_str(NTADR_A(2, 2), "STARTED BROS");

    ppu_on_all(); // enable rendering

    while(1) {
        ppu_wait_nmi();
        runApplications();
    }
}