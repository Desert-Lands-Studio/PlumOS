#include "plumlayout.h"
#include "core/plumallocator.h"
#include <stdlib.h>

static void abstract_layout_update(PlumLayout* self) {
    (void)self;
}

PlumLayout* plum_layout_new(void) {
    PlumLayout* layout = (PlumLayout*)plum_alloc(sizeof(PlumLayout));
    if (!layout) return NULL;

    layout->parent = NULL;
    layout->update = abstract_layout_update;

    return layout;
}
