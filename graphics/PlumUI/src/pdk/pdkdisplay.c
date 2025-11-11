#include "pdkdisplay.h"
#include "plumallocator.h"

bool pdk_display_open(PdkDisplay** out)
{ 
    *out = plum_alloc(sizeof(PdkDisplay)); 
    return true; 
}

void pdk_display_close(PdkDisplay* d)
{ 
    plum_free(d); 
}

PdkDisplay* pdk_display_default(void) {
    static PdkDisplay display = {0};
    return &display;
}