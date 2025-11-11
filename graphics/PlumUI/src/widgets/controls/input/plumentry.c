#include "plumentry.h"
#include "core/plumallocator.h"
#include <string.h>
#include <stdio.h>

static void entry_paint(PlumWidget* self, void* renderer) {
    PlumEntry* entry = (PlumEntry*)self;
    (void)renderer;
    
    const char* display_text = entry->text ? entry->text : "";
    printf("Entry [%s] at (%d, %d), focus: %s, cursor: %zu\n", 
           display_text, self->x, self->y, 
           entry->has_focus ? "YES" : "NO", entry->cursor_pos);
}

static void entry_on_event(PlumWidget* self, const PlumEvent* event) {
    PlumEntry* entry = (PlumEntry*)self;
    
    if (event->type == PLUM_EVENT_MOUSE_PRESS) {
        entry->has_focus = true;
        printf("Entry gained focus\n");
        plum_widget_update(self);
    }
    else if (event->type == PLUM_EVENT_KEY_PRESS && entry->has_focus) {
        if (event->data.key.key >= 32 && event->data.key.key <= 126) {
            char ch = (char)event->data.key.key;
            
            if (!entry->text) {
                entry->text = (char*)plum_alloc(2);
                if (entry->text) {
                    entry->text[0] = ch;
                    entry->text[1] = '\0';
                    entry->cursor_pos = 1;
                }
            } else {
                size_t len = strlen(entry->text);
                char* new_text = (char*)plum_alloc(len + 2);
                if (new_text) {
                    strncpy(new_text, entry->text, entry->cursor_pos);
                    new_text[entry->cursor_pos] = ch;
                    strcpy(new_text + entry->cursor_pos + 1, entry->text + entry->cursor_pos);
                    plum_free(entry->text);
                    entry->text = new_text;
                    entry->cursor_pos++;
                }
            }
            printf("Entry text: %s\n", entry->text);
            plum_widget_update(self);
        }
    }
}

PLUM_API PlumEntry* plum_entry_new(void) {
    PlumEntry* entry = (PlumEntry*)plum_alloc(sizeof(PlumEntry));
    if (!entry) return NULL;

    PlumWidget* base = plum_widget_new();
    if (!base) {
        plum_free(entry);
        return NULL;
    }
    memcpy(&entry->base, base, sizeof(PlumWidget));
    plum_free(base);

    entry->base.paint = entry_paint;
    entry->base.on_event = entry_on_event;
    
    entry->base.width = 120;
    entry->base.height = 25;

    entry->text = NULL;
    entry->cursor_pos = 0;
    entry->has_focus = false;

    return entry;
}

PLUM_API void plum_entry_set_text(PlumEntry* entry, const char* text) {
    if (!entry) return;

    if (entry->text) {
        plum_free(entry->text);
    }

    if (text) {
        size_t len = strlen(text) + 1;
        entry->text = (char*)plum_alloc(len);
        if (entry->text) {
            strncpy(entry->text, text, len);
            entry->cursor_pos = len - 1;
        }
    } else {
        entry->text = NULL;
        entry->cursor_pos = 0;
    }

    plum_widget_update((PlumWidget*)entry);
}

PLUM_API const char* plum_entry_get_text(PlumEntry* entry) {
    return entry ? entry->text : NULL;
}