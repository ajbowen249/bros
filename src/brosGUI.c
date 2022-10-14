#include "brosKernel.h"
#include "brosGUI.h"
#include "neslib.h"

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

    if (getPid == BROS_INVALID_PID) {
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

    label->pid = getPid;
    label->type = GCT_LABEL;
    label->isVisible = 1;
    label->control.label.maxLength = 0;
    label->control.label.value = nullptr;

    g_guiControls[controlIndex] = label;

    g_guiNeedsRefresh = 1;

    return 1;
}

void invalidate() {
    g_guiNeedsRefresh = 1;
}

void clearScreen() {
    // IMPROVE: This is SLOW and dumb. Attempts at just writing the block have failed.
    //          Must be a better way to do this.

    unsigned int y;

    for (y = 0; y < NAMETABLE_HEIGHT; ++y) {
        put_str(NTADR_A(0, y), "                                ");
    }
}

void drawLabel(GUIControl* label) {
    // unsigned char length = label->control.label.maxLength;
    // char* source = label->control.label.value;
    // unsigned int x = label->control.label.x;
    // unsigned int y = label->control.label.y;
    // unsigned char maxX = x + label->control.label.maxLength;
    // bool pastEnd = 0;
    // unsigned int index;

    // if (y > NAMETABLE_MAX_Y) {
    //     return;
    // }

    // if (maxX > NAMETABLE_MAX_X) {
    //     maxX = NAMETABLE_MAX_X;
    // }

    // if (x > maxX) {
    //     return;
    // }

    put_str(NTADR_A(label->control.label.x, label->control.label.y), label->control.label.value);
   return;

    // vram_adr(NTADR_A(x, y));

    // for (index = 0; index < length; ++index) {
    //     char ch;
    //     if (!pastEnd) {
    //         ch = source[index];
    //     } else {
    //         ch = " ";
    //     }

    //     if (!ch) {
    //         pastEnd = 1;
    //         ch = " ";
    //     }

    //     vram_put(ch - 0x20);

    //     ++x;
    //     if (x > maxX) {
    //         return;
    //     }
    // }
}

void processGUI() {
    unsigned int controlIndex;
    GUIControl* control;

    if (!g_guiNeedsRefresh) {
        return;
    }

    ppu_off();
    clearScreen();

    for (controlIndex = 0; controlIndex < MAX_GUI_CONTROLS; ++controlIndex) {
        control = g_guiControls[controlIndex];
        if (control && control->isVisible) {
            switch (control->type)
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

void guiNeedsRefresh() {
    g_guiNeedsRefresh = 1;
}
