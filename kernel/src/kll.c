#include "kll.h"

/*
Takes:
    - pointer to the list start (can also be NULL if no list exists)
    - pointer to already kmallcoed data

Returns:
    - pointer to list start
*/
kll_node* kllAddNode(kll_node* list_start, void* data){
    kll_node* new = (kll_node*)kmalloc(sizeof(kll_node));
    if (new == NULL){
        biosTermPrintf("ERR: Kmalloc\n");
        return NULL;
    }
    new->data = data;

    if(list_start == NULL){
        new->next = NULL;
        new->prev = NULL;
        list_start = new;
    }
    else{
        kll_node* curr = list_start;
        while (curr->next != NULL){
            curr = curr->next;
        }
        curr->next = new;
        new->prev = curr;
        new->next = NULL;
    }
    return list_start;
}

/*
Takes: pointer to list start
Returns: Number of nodes
*/
unsigned int kllGetLength(kll_node* list_start){
    unsigned int length = 0;
    while (list_start != NULL){
        list_start = list_start->next;
        length++;
    }
    return length;
}

/*
Takes:
    - pointer to list start
    - index of node

Returns:
    - pointer to the data of that node
*/
void* kllGetData(kll_node* list_start, unsigned int index){
    if (list_start == NULL){
        return NULL;
    }

    for (int i = index; i > 0; i--){
        list_start = list_start->next;
        if(list_start == NULL){
            biosTermPrintf("ERR: KLL index out of range");
            return NULL;
        }
    }
    return list_start->data;
}

/*
Takes:
    - pointer to the list start (can also be NULL if no list exists)
    - index of node to be deleted

Returns:
    - pointer to list start
*/
kll_node* kllDeleteNode(kll_node* list_start, unsigned int index){
    if (list_start == NULL){
        return NULL;
    }
    
    kll_node* curr = list_start;
    
    for (int i = index; i > 0; i--){
        curr = curr->next;
        if(curr == NULL){
            biosTermPrintf("ERR: KLL index out of range");
            return list_start;
        }
    }

    // only node
    if (curr->prev == NULL && curr->next == NULL){
        list_start = NULL;
    }
    // first node
    else if (curr->prev == NULL){
        list_start = curr->next;
        list_start->prev = NULL;
    }
    // last node
    else if (curr->next == NULL){
        curr->prev->next = NULL;
    }
    // middle node
    else{
        curr->next->prev = curr->prev;
        curr->prev->next = curr->next;
    }

    kfree(curr->data);
    kfree(curr);

    return list_start;
}
