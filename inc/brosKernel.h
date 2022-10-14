#ifndef BROS_KERNEL_H
#define BROS_KERNEL_H

#include "bros.h"
#include "neslib.h"

// TODO: These eventually need to be declared in a way where we know how to call them from apps

void __fastcall__ setSystemError(SystemError err);

// put a string into the nametable
void __fastcall__ put_str(unsigned int adr, const char* str);

// Exits the current process
void exitCurrentProcess();

bool allocateProcess(const ApplicationHeader* header, unsigned int fileSize);

void runApplications();

void initializeTable();

unsigned int getPid();

ProcessTable* getProcessTable();

#endif
