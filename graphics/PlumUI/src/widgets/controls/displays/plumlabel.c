#include "plumlabel.h"
#include "core/plumallocator.h"
#include <string.h>
#include <stdio.h>


static void label_paint(PlumWidget* self, void* renderer) {
    PlumLabel* label = (PlumLabel*)self;
    (void)renderer; 

    if (label->text) {
        printf("Label at (%d, %d): \"%s\"\n", self->x, self->y, label->text);
    }
}

PLUM_API PlumLabel* plum_label_new(const char* text) {
    PlumLabel* label = (PlumLabel*)plum_alloc(sizeof(PlumLabel));
    if (!label) return NULL;

    
    PlumWidget* base_widget = plum_widget_new();
    if (!base_widget) {
        plum_free(label);
        return NULL;
    }
    
    
    memcpy(&label->base, base_widget, sizeof(PlumWidget));
    plum_free(base_widget);

    
    label->base.paint = label_paint;

    
    label->text = NULL;
    plum_label_set_text(label, text);

    return label;
}


PLUM_API void plum_label_set_text(PlumLabel* label, const char* text) {
    if (!label) return;

    if (label->text) {
        plum_free(label->text);
    }

    if (text) {
        size_t len = strlen(text) + 1;
        label->text = (char*)plum_alloc(len);
        if (label->text) {
            strncpy(label->text, text, len);
        }
    } else {
        label->text = NULL;
    }

    plum_widget_update((PlumWidget*)label);
}
