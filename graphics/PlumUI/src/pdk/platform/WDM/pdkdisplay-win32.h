#include "pdkdisplay.h"
#include "core/plumallocator.h"
#include <windows.h>
#include <stdio.h>

PdkDisplay* pdk_display_default(void) {
    
    
    static PdkDisplay display = {0};
    
    printf("✅ Win32 display initialized\n");
    return &display;
}

void pdk_display_close(PdkDisplay* display) {
    
    (void)display;
}
