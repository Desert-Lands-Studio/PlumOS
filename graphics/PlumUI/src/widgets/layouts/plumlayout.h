#ifndef PLUM_LAYOUT_H
#define PLUM_LAYOUT_H

#include "widgets/base/widget/plumwidget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PlumLayout PlumLayout;

struct PlumLayout {
    PlumWidget* parent;
    void (*update)(PlumLayout* self);
};

PlumLayout* plum_layout_new(void);

#ifdef __cplusplus
}
#endif

#endif /* PLUM_LAYOUT_H */
