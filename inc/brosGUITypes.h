#ifndef BROS_GUI_TYPES_H
#define BROS_GUI_TYPES_H

typedef enum _GUIControlType {
    GCT_NOTHING = 0,
    GCT_LABEL = 1,
} GUIControlType;

/**
 * Literally nothing. Here to prove the union approach works.
 */
typedef struct _GUINothing {
    int dummy;
} GUINothing;

/**
 * A single-line limited-length string of text on the screen
 */
typedef struct _GUILabel {
    unsigned char maxLength;
    char* value;
    unsigned int x;
    unsigned int y;
} GUILabel;

/**
 * The union of all possible controls
 */
typedef union _GUIControlUnion {
    GUINothing nothing;
    GUILabel label;
} GUIControlUnion;

/**
 * The core GUI object type for the registry
 */
typedef struct _GUIControl {
    GUIControlType type;
    unsigned int pid;
    bool isVisible;
    GUIControlUnion control;
} GUIControl;

typedef enum _GUIError {
    GE_SUCCESS = 0,

    GE_OUT_OF_MEMORY = 1,
    GE_NOT_CALLED_FROM_APP = 2,
} GUIError;

#define MAX_GUI_CONTROLS 50

#endif
