#ifndef PLUM_ENTRY_H
#define PLUM_ENTRY_H

#include "widgets/base/widget/plumwidget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PlumEntry PlumEntry;

struct PlumEntry {
    PlumWidget base;
    char* text;
    size_t cursor_pos;
    bool has_focus;
};

PLUM_API PlumEntry* plum_entry_new(void);
PLUM_API void plum_entry_set_text(PlumEntry* entry, const char* text);
PLUM_API const char* plum_entry_get_text(PlumEntry* entry);

#ifdef __cplusplus
}
#endif

#endif /* PLUM_ENTRY_H */
