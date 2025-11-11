#pragma once
#include <stdbool.h>
#include "pdktypes.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PdkDisplay { 
    int dummy; 
    
    
};

bool pdk_display_open(PdkDisplay** out);
void pdk_display_close(PdkDisplay*);
PdkDisplay* pdk_display_default(void);  

#ifdef __cplusplus
}
#endif