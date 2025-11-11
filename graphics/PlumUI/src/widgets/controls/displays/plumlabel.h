/* ── PlumX/PlumUI/src/widgets/plumlabel.h ─────────────────────────────────────────── */
#ifndef PLUM_LABEL_H
#define PLUM_LABEL_H

#include "widgets/base/widget/plumwidget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PlumLabel PlumLabel;

struct PlumLabel {
    PlumWidget base;
    char* text;
    
};

PLUM_API PlumLabel* plum_label_new(const char* text);
PLUM_API void plum_label_set_text(PlumLabel* label, const char* text);

#ifdef __cplusplus
}
#endif

#endif /* PLUM_LABEL_H */
