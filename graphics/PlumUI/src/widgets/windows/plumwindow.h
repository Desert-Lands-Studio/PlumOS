#ifndef PLUM_WINDOW_H
#define PLUM_WINDOW_H

#include "widgets/base/widget/plumwidget.h"
#include "pdk/pdkwindow.h"
#include "psk/psk.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PlumWindow PlumWindow;

struct PlumWindow {
    PlumWidget base;
    PdkWindow* pdk_window;
    PskRenderer* renderer;
    void (*on_event)(PlumWindow* self, const PlumEvent* event);
    bool is_running;
    bool is_fullscreen;
};

PLUM_API PlumWindow* plum_window_create(const char* title, int width, int height);
PLUM_API void plum_window_destroy(PlumWindow* window);
PLUM_API void plum_window_set_root_widget(PlumWindow* window, PlumWidget* root);
PLUM_API void plum_window_run(PlumWindow* window);

#ifdef __cplusplus
}
#endif

#endif /* PLUM_WINDOW_H */
