#ifndef PDK_WINDOW_H
#define PDK_WINDOW_H

#include "plumconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PdkWindow PdkWindow;

PLUM_API PdkWindow* pdk_window_create(const char* title, int width, int height);
PLUM_API void pdk_window_destroy(PdkWindow* window);
PLUM_API void* pdk_window_poll_events(PdkWindow* window);

#ifdef __cplusplus
}
#endif

#endif 