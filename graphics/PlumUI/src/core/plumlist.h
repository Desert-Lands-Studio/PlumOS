#ifndef PLUM_LIST_H
#define PLUM_LIST_H

#include <stddef.h>
#include <stdbool.h>

typedef struct PlumListNode PlumListNode;
typedef struct PlumList PlumList;

typedef int (*PlumListCompareFunc)(void* elementa, void* elementb);

PlumList* plum_list_new(void);
void plum_list_free(PlumList* list);
size_t plum_list_size(const PlumList* list);
void* plum_list_get(const PlumList* list, size_t index);

PlumListNode* plum_list_append(PlumList* list, void* data);
PlumListNode* plum_list_prepend(PlumList* list, void* data);
void plum_list_remove(PlumList* list, void* data);

void* plum_list_get_first(const PlumList* list);
void* plum_list_get_last(const PlumList* list);
void* plum_list_get_nth(const PlumList* list, size_t n);

void plum_list_sort(PlumList* list, PlumListCompareFunc compare_func);

#endif /* PLUM_LIST_H */
