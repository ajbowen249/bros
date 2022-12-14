#ifndef BROS_GUI_H
#define BROS_GUI_H

#include "bros.h"

// This is a good candidate for a "system app." It will inevitably outgrow the kernel, and is definitely something I'll
// want to make updateable.

// TODO: These are callable the same as kernel functions for now. If the GUI server becomes a system app, I'll
//       want a layer of indirection, like a named pipe, or something.

// Note: While calling into this is a TODO, callbacks from this should come in the form of another app lifecycle
//       method.

// Note: We really want dynamic memory in the future. For now, the comprimise is that applications are responsible
//       for allocating space for controls in their own memory, but there is still a fixed overall number of controls
//       the GUI server can keep track of in its registry.

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

/**
 * Initializes and registers a label
 */
bool addLabel(GUIControl* label);

/**
 * Invalidate the ~current process's~ GUI
*/
void invalidate();

void initGui();

void processGUI();

void guiNeedsRefresh();

typedef enum _GUIError {
    GE_SUCCESS = 0,

    GE_OUT_OF_MEMORY = 1,
    GE_NOT_CALLED_FROM_APP = 2,
} GUIError;

#define MAX_GUI_CONTROLS 50

#endif
