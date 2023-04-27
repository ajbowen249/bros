#ifndef BROS_GUI_H
#define BROS_GUI_H

#include "bros.h"
#include "brosGUITypes.h"

// This is a good candidate for a "system app." It will inevitably outgrow the kernel, and is definitely something I'll
// want to make updateable.

// TODO: These are callable the same as kernel functions for now. If the GUI server becomes a system app, I'll
//       want a layer of indirection, like a named pipe, or something.

// Note: While calling into this is a TODO, callbacks from this should come in the form of another app lifecycle
//       method.

// Note: We really want dynamic memory in the future. For now, the comprimise is that applications are responsible
//       for allocating space for controls in their own memory, but there is still a fixed overall number of controls
//       the GUI server can keep track of in its registry.


/**
 * Initializes and registers a label
 */
bool addLabel(GUIControl* label);

void initGui();

void processGUI();

void guiNeedsRefresh();

#endif
