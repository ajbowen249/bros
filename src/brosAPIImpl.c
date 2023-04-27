#include "brosAPIImpl.h"
#include "bros.h"
#include "brosAPI.h"
#include "brosKernel.h"
#include "brosGUI.h"
#include "brosFS.h"

#pragma bss-name(push, "API_DESC")
APIDescriptor* DescriptorPtr;
#pragma bss-name(pop)

APIDescriptor Descriptor;

void initApi() {
    Descriptor.version = BROS_API_HEADER_VERSION;

    Descriptor.exitCurrentProcess = &exitCurrentProcess;
    Descriptor.getPid = &getPid;

    Descriptor.gui_addLabel = &addLabel;
    Descriptor.gui_guiNeedsRefresh = &guiNeedsRefresh;

    Descriptor.fs_listFolder = &listFolder;
    Descriptor.fs_openFile = &openFile;
    Descriptor.fs_readBlock = &readBlock;
    Descriptor.fs_getFileBlockSize = &getFileBlockSize;

    DescriptorPtr = &Descriptor;
}
