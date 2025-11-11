#include "plumbutton.h"
#include "core/plumallocator.h"
#include <string.h>
#include <stdio.h>

static void button_paint(PlumWidget* self, void* renderer) {
    PlumButton* button = (PlumButton*)self;
    (void)renderer; 
    
    if (button->text) {
        printf("Button [%s] at (%d, %d) size: %dx%d, pressed: %s\n", 
               button->text, self->x, self->y, self->width, self->height,
               button->is_pressed ? "YES" : "NO");
    }
}

static void button_on_event(PlumWidget* self, const PlumEvent* event) {
    PlumButton* button = (PlumButton*)self;
    
    if (event->type == PLUM_EVENT_MOUSE_PRESS) {
        button->is_pressed = true;
        printf("Button pressed: %s\n", button->text);
        plum_widget_update(self);
    } 
    else if (event->type == PLUM_EVENT_MOUSE_RELEASE) {
        if (button->is_pressed) {
            button->is_pressed = false;
            printf("Button released: %s\n", button->text);
            if (button->on_click) {
                button->on_click(button);
            }
            plum_widget_update(self);
        }
    }
}

PLUM_API PlumButton* plum_button_new(const char* text) {
    PlumButton* button = (PlumButton*)plum_alloc(sizeof(PlumButton));
    if (!button) return NULL;

    
    PlumWidget* base = plum_widget_new();
    if (!base) {
        plum_free(button);
        return NULL;
    }
    memcpy(&button->base, base, sizeof(PlumWidget));
    plum_free(base);

    
    button->base.paint = button_paint;
    button->base.on_event = button_on_event;
    
    
    button->base.width = 80;
    button->base.height = 30;

    button->text = NULL;
    plum_button_set_text(button, text);
    button->on_click = NULL;
    button->is_pressed = false;

    return button;
}

PLUM_API void plum_button_set_text(PlumButton* button, const char* text) {
    if (!button) return;

    if (button->text) {
        plum_free(button->text);
    }

    if (text) {
        size_t len = strlen(text) + 1;
        button->text = (char*)plum_alloc(len);
        if (button->text) {
            strncpy(button->text, text, len);
        }
    } else {
        button->text = NULL;
    }

    plum_widget_update((PlumWidget*)button);
}

PLUM_API const char* plum_button_get_text(PlumButton* button) {
    return button ? button->text : NULL;
}

PLUM_API void plum_button_set_on_click(PlumButton* button, void (*callback)(PlumButton*)) {
    if (button) {
        button->on_click = callback;
    }
}