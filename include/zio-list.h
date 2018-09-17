#ifndef ZIO_LIST_H_INCLUDED
#define ZIO_LIST_H_INCLUDED

#include <stdlib.h>
#include <string.h>

#define _NOMAIN_ //necessary to avoid multiple definitions of "debug" from con_interaction
#include "zio-con.h"
#undef _NOMAIN_


typedef struct kostring {
    size_t length;
    size_t allocated_length;
    size_t block_bytes;
    char* cstring;
} kostring;

/* "length" is the ammount of integers that "integers" can currently hold */
typedef struct intlist {
    size_t length;
    size_t allocated_length;
    size_t block_ints;
    int* integers;
} intlist;

void kostring_zero(kostring*);
void kostring_write(kostring*, char*);
void kostring_resize(kostring*, size_t);
void kostring_append(kostring*, char);
void kostring_free(kostring*);

/* intlist */
void intlist_zero(intlist*);
void intlist_write(intlist*, char*);
void intlist_append(intlist*, int);
void intlist_resize(intlist*, size_t);
void intlist_free(intlist*);

/* pointer-list */
typedef struct ptrlist {
    size_t length;
    size_t allocated_length;
    size_t block_ptrs;
    void** pointers;
} ptrlist;


/* typeless list */
typedef struct alist {
    size_t length; /*how many units the list currently holds*/
    size_t allocated_length; /*how many units the list can maximally hold atm*/
    size_t block_units;  /*how many units should be allocated at once (TODO: if zero use a precentage of its size(like 200%))*/
    size_t usize; /*how many chars one unit is big*/
    void* start_ptr; /*first unit of the list*/
} alist;

void alist_zero(alist*); /*like doing =0 in a primitive*/
void alist_cat(alist*, alist*); /*TODO: concaterate two lists in some way*/
void alist_append(alist*, void* unit_ptr); /*add the unit pointed at by unit_ptr to the list, unit size should be the same as the list's usize*/
void alist_resize(alist*, size_t); /*make the list able to contain size_t units*/
void alist_free(alist*); /*just like free() for normal pointers*/

#define alist_create_type(Alist, Typ, Blocksize) ({alist_zero(Alist); alist_maketype(&Alist, Typ); Alist.block_units = Blocksize})
#define alist_maketype(Alist, Typ) (Alist.usize = sizeof(Typ))
#define alist_append_type(Alist, Typ, Var) ({Typ Variable = Var; alist_append(Alist, &Variable)})


#endif // ZIO_LIST_H_INCLUDED
