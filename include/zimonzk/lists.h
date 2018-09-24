/** @file
 *  \brief interface to the lists
 *
 */
#ifndef LISTS_H_INCLUDED
#define LISTS_H_INCLUDED

#include <stdint.h>
#include <stdlib.h>

typedef struct arraylist_s
{
    void* data;
    uint64_t used_units;
    uint64_t allocated_units;
    uint16_t allocation_block_units;
    uint16_t unit_size;
} arraylist;

/** \brief Initializes a list before it can be used
 *
 * @param list [in] pointer to the list, that should be initialized
 */
void arraylist_init(arraylist* list, uint16_t unit_size, uint16_t allocation_block_units);

void arraylist_delete(arraylist* list);

void arraylist_append(arraylist* list, void* element_ptr);

void arraylist_instert(arraylist* list, void* element_ptr, uint64_t index);

void arraylist_del_element(arraylist* list, uint64_t index);

void arraylist_replace(arraylist* list, void* element_ptr, uint64_t index);

void* arraylist_get(arraylist* list, uint64_t index);

void arraylist_append_multi(arraylist* list, void* element0_ptr, uint64_t ammount);

void arraylist_insert_multi(arraylist* list, void* element0_ptr, uint64_t index, uint64_t ammount);

void arraylist_replace_multi(arraylist* list, void* element0_ptr, uint64_t index, uint64_t ammount);

#endif
