#include "brosKernel.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

// The actual process table
ProcessTable g_processTable;

// The ID of the process currently being called
unsigned int g_pid;

// The last system error
SystemError g_systemError;

void setSystemError(SystemError err) {
    g_systemError = err;
}

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
        unsigned int location = offset + BROS_APP_RAM_START;
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

        process->location = location;
        process->size = memorySize;

        process->start = header->startOffset ? location + header->startOffset : 0;
        process->proc = header->procOffset ? location + header->procOffset : 0;
        process->onExit = header->exitOffset ? location + header->exitOffset : 0;
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

    g_pid = BROS_INVALID_PID;
}

unsigned int getPid() {
    return g_pid;
}

ProcessTable* getProcessTable() {
    return &g_processTable;
}

int b_sprintf(char* buf, const char* fmt, ...) {
    int ret;
    va_list args;
    va_start(args, fmt);
    ret = vsprintf(buf, fmt, args);
    va_end(args);

    return ret;
}
