#include "plumwidget.h"
#include "plumallocator.h"
#include "plumlist.h"
#include "layouts/plumlayout.h" 
#include <string.h>


static void default_paint(PlumWidget* self, void* renderer) {
    
    (void)self;
    (void)renderer;
}

static void default_on_event(PlumWidget* self, const PlumEvent* event) {
    
    (void)self;
    (void)event;
}

static void default_layout_update(PlumWidget* self) {
    
    if (self->layout && self->layout->update) {
        self->layout->update(self->layout);
    }
}

static void default_update(PlumWidget* self) {
    
    
    if (self->paint) {
        
        
    }
}

PLUM_API PlumWidget* plum_widget_new(void) {
    PlumWidget* widget = (PlumWidget*)plum_alloc(sizeof(PlumWidget));
    if (!widget) return NULL;

    
    memset(&widget->base, 0, sizeof(PlumObject));
    widget->base.ref_count = 1;

    
    widget->x = 0;
    widget->y = 0;
    widget->width = 100;
    widget->height = 30;
    widget->visible = true;
    widget->enabled = true;

    
    widget->parent = NULL;
    widget->children = plum_list_new();
    widget->layout = NULL;

    
    widget->paint = default_paint;
    widget->on_event = default_on_event;
    widget->layout_update = default_layout_update;
    widget->update = default_update;

    
    widget->userdata = NULL;

    return widget;
}

PLUM_API void plum_widget_destroy(PlumWidget* widget) {
    if (!widget) return;

    
    if (widget->children) {
        while (plum_list_size(widget->children) > 0) {
            PlumWidget* child = (PlumWidget*)plum_list_get(widget->children, 0);
            plum_widget_remove_child(widget, child);
        }
        plum_list_free(widget->children);
    }

    if (widget->layout) {
        plum_free(widget->layout);  
    }
    if (widget->userdata) {
        plum_free(widget->userdata);
    }

    
    plum_free(widget);
}


PLUM_API void plum_widget_add_child(PlumWidget* parent, PlumWidget* child) {
    if (!parent || !child) return;

    
    if (child->parent) {
        plum_widget_remove_child(child->parent, child);
    }

    
    child->parent = parent;
    plum_list_append(parent->children, child);

    
    child->x = 0;
    child->y = 0;
    

    
    if (parent->layout_update) {
        parent->layout_update(parent);
    }
}


PLUM_API void plum_widget_remove_child(PlumWidget* parent, PlumWidget* child) {
    if (!parent || !child || child->parent != parent) return;

    plum_list_remove(parent->children, child);
    child->parent = NULL;

    if (parent->layout_update) {
        parent->layout_update(parent);
    }
}


PLUM_API void plum_widget_set_geometry(PlumWidget* widget, int x, int y, int width, int height) {
    if (!widget) return;
    widget->x = x;
    widget->y = y;
    widget->width = width;
    widget->height = height;
    plum_widget_update(widget); 
}


PLUM_API void plum_widget_set_visible(PlumWidget* widget, bool visible) {
    if (!widget || widget->visible == visible) return;
    widget->visible = visible;
    plum_widget_update(widget);
}


PLUM_API void plum_widget_set_enabled(PlumWidget* widget, bool enabled) {
    if (!widget || widget->enabled == enabled) return;
    widget->enabled = enabled;
    plum_widget_update(widget);
}


PLUM_API void plum_widget_update(PlumWidget* widget) {
    if (!widget || !widget->visible || !widget->paint) return;
    
    
    
    
}


PLUM_API void plum_widget_dispatch_event(PlumWidget* widget, const PlumEvent* event) {
    if (!widget || !event) return;

    
    if (widget->on_event && widget->enabled) {
        widget->on_event(widget, event);
    }

    
    if (widget->children) {
        for (int i = plum_list_size(widget->children) - 1; i >= 0; i--) {
            PlumWidget* child = (PlumWidget*)plum_list_get(widget->children, i);
            if (child && child->visible) {
                
                bool in_bounds = true;
                if (event->type == PLUM_EVENT_MOUSE_PRESS ||
                    event->type == PLUM_EVENT_MOUSE_RELEASE ||
                    event->type == PLUM_EVENT_MOUSE_MOVE) {
                    int local_x = event->data.mouse.x - child->x;
                    int local_y = event->data.mouse.y - child->y;
                    in_bounds = (local_x >= 0 && local_x < child->width &&
                                 local_y >= 0 && local_y < child->height);
                }

                if (in_bounds) {
                    PlumEvent local_event = *event;
                    
                    if (event->type == PLUM_EVENT_MOUSE_PRESS ||
                        event->type == PLUM_EVENT_MOUSE_RELEASE ||
                        event->type == PLUM_EVENT_MOUSE_MOVE) {
                        local_event.data.mouse.x -= child->x;
                        local_event.data.mouse.y -= child->y;
                    }
                    plum_widget_dispatch_event(child, &local_event);
                }
            }
        }
    }
}