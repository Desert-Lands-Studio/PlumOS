#include "plumlist.h"
#include "plumallocator.h"
#include <stdlib.h>

typedef struct PlumListNode {
    void* data;
    struct PlumListNode* next;
    struct PlumListNode* prev;
} PlumListNode;

struct PlumList {
    PlumListNode* head;
    PlumListNode* tail;
    size_t size;
};

PlumList* plum_list_new(void) {
    PlumList* list = plum_alloc(sizeof(PlumList));
    if (list) {
        list->head = list->tail = NULL;
        list->size = 0;
    }
    return list;
}

void plum_list_free(PlumList* list) {
    if (!list) return;
    
    PlumListNode* current = list->head;
    while (current) {
        PlumListNode* next = current->next;
        plum_free(current);
        current = next;
    }
    plum_free(list);
}

size_t plum_list_size(const PlumList* list) {
    return list ? list->size : 0;
}

void* plum_list_get(const PlumList* list, size_t index) {
    if (!list || index >= list->size) return NULL;
    
    PlumListNode* current = list->head;
    for (size_t i = 0; i < index && current; i++) {
        current = current->next;
    }
    return current ? current->data : NULL;
}

PlumListNode* plum_list_append(PlumList* list, void* data) {
    if (!list) return NULL;
    
    PlumListNode* new_node = plum_alloc(sizeof(PlumListNode));
    if (!new_node) return NULL;
    
    new_node->data = data;
    new_node->next = NULL;
    new_node->prev = list->tail;
    
    if (list->tail) {
        list->tail->next = new_node;
    } else {
        list->head = new_node;
    }
    
    list->tail = new_node;
    list->size++;
    
    return new_node;
}

void plum_list_remove(PlumList* list, void* data) {
    if (!list || !data) return;
    
    PlumListNode* current = list->head;
    while (current) {
        if (current->data == data) {
            
            if (current->prev) {
                current->prev->next = current->next;
            } else {
                list->head = current->next;
            }
            
            if (current->next) {
                current->next->prev = current->prev;
            } else {
                list->tail = current->prev;
            }
            
            plum_free(current);
            list->size--;
            return;
        }
        current = current->next;
    }
}