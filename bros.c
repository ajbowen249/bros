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

void main(void) {
    g_pid = BROS_INVALID_PID;
    setSystemError(SERR_SUCCESS);
    // rendering is disabled at the startup, the palette is all black

    pal_col(1, 0x30); // set while color

    // you can't put data into vram through vram_put while rendering is enabled
    // so you have to disable rendering to put things like text or a level map
    // into the nametable

    // there is a way to update small number of nametable tiles while rendering
    // is enabled, using set_vram_update and an update list

    put_str(NTADR_A(2, 2), "HELLO, WORLD!");
    put_str(NTADR_A(2, 6), "THIS CODE PRINTS SOME TEXT");
    put_str(NTADR_A(2, 7), "USING ASCII-ENCODED CHARSET");
    put_str(NTADR_A(2, 8), "(WITH CAPITAL LETTERS ONLY)");
    put_str(NTADR_A(2, 10), "TO USE CHR MORE EFFICIENTLY");
    put_str(NTADR_A(2, 11), "YOU'D NEED A CUSTOM ENCODING");
    put_str(NTADR_A(2, 12), "AND A CONVERSION TABLE");

    put_str(NTADR_A(2, 16), "CURRENT VIDEO MODE IS");

    if (ppu_system()) {
        put_str(NTADR_A(24, 16), "NTSC");
    } else {
        put_str(NTADR_A(24, 16), "PAL");
    }

    ppu_on_all(); // enable rendering

    while(1) {
        ppu_wait_nmi();
        runApplications();
    }
}