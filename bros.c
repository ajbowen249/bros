#include "bros.h"

#include "neslib.h"

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

        process->location = offset;
        process->size = memorySize;

        process->start = header->startOffset ? WRAM_START + offset + header->startOffset : 0;
        process->proc = header->procOffset ? WRAM_START + offset + header->procOffset : 0;
        process->onExit = header->exitOffset ? WRAM_START + offset + header->exitOffset : 0;
        process->state = PS_LOADING;
        put_str(NTADR_A(2, 3), "SCHEDULED");

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
            put_str(NTADR_A(2, 16), "CALL START");
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

int __fastcall__ doSomething();
int doSomething() {
    put_str(NTADR_A(2, 4), "ACTUALLY CALLED");
    return 0;
}

unsigned char* g_workRam = WRAM_START;

void main(void) {
    unsigned int testAppIdx;
    unsigned int doSomethingAddr = (unsigned int)doSomething;

    unsigned char TestApplication1[19] = {
        'B', 'R', 'O', 'S',
        0x01, 0x00, // ABI version
        0x00, 0x00, // No labels
        0x00, 0x00, // No extra ram
        0x10, 0x00, // Start at offset 16
        0x00, 0x00, // No proc
        0x00, 0x00, // No exit hook
    };

    TestApplication1[16] = 0x20;
    TestApplication1[17] = (unsigned char)doSomethingAddr;
    TestApplication1[18] = (unsigned char)(doSomethingAddr >> 8);

    initializeTable();

    g_pid = BROS_INVALID_PID;
    setSystemError(SERR_SUCCESS);
    pal_col(1, 0x30); // set while color


    allocateProcess((ApplicationHeader*)TestApplication1, 19);
    for (testAppIdx = 0; testAppIdx < 19; ++testAppIdx) {
        unsigned int* target = WRAM_START + testAppIdx;
        *(target) = TestApplication1[testAppIdx];
    }

    g_processTable.processes[0].state = PS_LOADED;

    // rendering is disabled at the startup, the palette is all black


    // you can't put data into vram through vram_put while rendering is enabled
    // so you have to disable rendering to put things like text or a level map
    // into the nametable

    // there is a way to update small number of nametable tiles while rendering
    // is enabled, using set_vram_update and an update list

    put_str(NTADR_A(2, 2), "INIT TEXT");

    ppu_on_all(); // enable rendering

    while(1) {
        ppu_wait_nmi();
        runApplications();
    }
}