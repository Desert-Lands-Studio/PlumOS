
#ifndef PLUM_OBJECT_H
#define PLUM_OBJECT_H

#include <stddef.h>

typedef struct PlumObject PlumObject;

struct PlumObject {
    int ref_count;
    void* user_data;
    void (*free_func)(void*);
};

#endif /* PLUM_OBJECT_H */
