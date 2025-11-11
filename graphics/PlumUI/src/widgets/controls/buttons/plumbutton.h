#ifndef PLUM_BUTTON_H
#define PLUM_BUTTON_H

#include "widgets/base/widget/plumwidget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PlumButton PlumButton;

struct PlumButton {
    PlumWidget base;
    char* text;
    void (*on_click)(PlumButton* self);
    bool is_pressed;
};

PLUM_API PlumButton* plum_button_new(const char* text);
PLUM_API void plum_button_set_text(PlumButton* button, const char* text);
PLUM_API const char* plum_button_get_text(PlumButton* button);
PLUM_API void plum_button_set_on_click(PlumButton* button, void (*callback)(PlumButton*));

#ifdef __cplusplus
}
#endif

#endif /* PLUM_BUTTON_H */