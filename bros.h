#ifndef BROS_H
#define BROS_H

// The maximum number of processes available in the BOS process table
#define MAX_BROS_PROCESSES 32

// Marker for an invalid process ID
#define BROS_INVALID_PID 255

// The total size of available work ram
#define BROS_APPLICATION_RAM_SIZE 8192

// shorthand for people used to using bool
#define bool unsigned char

#define WRAM_START 0x6000

// Errors that happen at the system level
typedef enum _SystemError {
    // NO error happened
    SERR_SUCCESS = 0,

    // Process Loading Errors

    // The application header didn't start with BROS
    SERR_INVALID_APPLICATION_HEADER = 1,

    // Could not find a continguous memory block big enough to run the application
    SERR_NOT_ENOUGH_MEMORY = 2,
} SystemError;

// The first bytes of any application file
typedef struct _ApplicationHeader {
    // The first four characters should be "BROS"

    // The "B" character marker
    char b;

    // The "R" character marker
    char r;

    // The "O" character marker
    char o;

    // The "S" character marker
    char s;

    // Which BOS version was this compiled for
    unsigned char abiVersion;

    // How many labels are in the label section
    unsigned int labelCount;

    // How much extra space should be added to the end of the loaded process in memory for variables
    unsigned int ramSize;

    // Offset of the start function
    unsigned int startOffset;

    // Offset of the proc function
    unsigned int procOffset;

    // Offset of the exit function
    unsigned int exitOffset;
} ApplicationHeader;

// The state of a running process
typedef enum _ProcessState {
    // No proces is active here
    PS_INACTIVE = 0,

    // The process has been allocated in the table, but not loaded into memory
    PS_LOADING = 1,

    // The process has been loaded, but start() has not been called
    PS_LOADED = 2,

    // start() has been called, and process is active
    PS_RUNNING = 3,

    // The application should exit
    PS_AWAITING_EXIT = 4,
} ProcessState;

// Describes a running process in memory
typedef struct _Process {
    // The state of the process
    ProcessState state;

    // its offet in application ram
    unsigned int location;

    // its size in applcation ram
    unsigned int size;

    void (*start)();
    void (*proc)();
    void (*onExit)();
} Process;

// Contains information about running processes
typedef struct _ProcessTable {
    // Information about all processes
    Process processes[MAX_BROS_PROCESSES];
} ProcessTable;

#endif
