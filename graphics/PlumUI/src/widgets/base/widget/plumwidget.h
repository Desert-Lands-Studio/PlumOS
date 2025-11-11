#ifndef PLUM_WIDGET_H
#define PLUM_WIDGET_H

#include <stdbool.h>
#include "plumconfig.h"
#include "core/plumobject.h"
#include "core/plumlist.h"
#include "render/rhi.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct PlumWidget PlumWidget;
typedef struct PlumEvent PlumEvent;
typedef struct PlumLayout PlumLayout;


typedef enum {
    PLUM_EVENT_NONE = 0,
    PLUM_EVENT_MOUSE_PRESS,
    PLUM_EVENT_MOUSE_RELEASE,
    PLUM_EVENT_MOUSE_MOVE,
    PLUM_EVENT_KEY_PRESS,
    PLUM_EVENT_KEY_RELEASE,
    PLUM_EVENT_WINDOW_RESIZE,
    PLUM_EVENT_PAINT
} PlumEventType;


struct PlumEvent {
    PlumEventType type;
    union {
        struct { int x, y; int button; } mouse;
        struct { int key; int modifiers; } key;
        struct { int width, height; } window;
    } data;
};


struct PlumWidget {
    PlumObject base; 

    
    int x, y, width, height;
    bool visible;
    bool enabled;

    
    PlumWidget* parent;
    PlumList* children; 

    
    PlumLayout* layout;

    
    void (*paint)(PlumWidget* self, void* renderer);
    void (*on_event)(PlumWidget* self, const PlumEvent* event);
    void (*layout_update)(PlumWidget* self);
    void (*update)(PlumWidget* self);

    
    void* userdata;
};


PLUM_API PlumWidget* plum_widget_new(void);
PLUM_API void plum_widget_destroy(PlumWidget* widget);

PLUM_API void plum_widget_add_child(PlumWidget* parent, PlumWidget* child);
PLUM_API void plum_widget_remove_child(PlumWidget* parent, PlumWidget* child);

PLUM_API void plum_widget_set_geometry(PlumWidget* widget, int x, int y, int width, int height);
PLUM_API void plum_widget_set_visible(PlumWidget* widget, bool visible);
PLUM_API void plum_widget_set_enabled(PlumWidget* widget, bool enabled);

PLUM_API void plum_widget_update(PlumWidget* widget);
PLUM_API void plum_widget_dispatch_event(PlumWidget* widget, const PlumEvent* event);

#ifdef __cplusplus
}
#endif

#endif /* PLUM_WIDGET_H */
