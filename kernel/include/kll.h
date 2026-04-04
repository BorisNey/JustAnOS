#pragma once

#include <stdint.h>
#include <stddef.h>
#include "kmalloc.h"
#include "util.h"

typedef struct kll_node{
    void* data;
    struct kll_node* next;
    struct kll_node* prev;
} kll_node;

kll_node* kllAddNode(kll_node* list_start, void* data);
unsigned int kllGetLength(kll_node* list_start);
void* kllGetData(kll_node* list_start, unsigned int index);
kll_node* kllDeleteNode(kll_node* list_start, unsigned int index);
