#ifndef DATATYPES_H
#define DATATYPES_H

#include <stdint.h>

typedef struct {
    uint32_t len;
    void **data;
} ptr_array_t;

typedef struct {
    uint32_t len;
    uint32_t *data;
} uint_array_t;

typedef struct {
    uint32_t len;
    char **keys;
    void **vals;
} table_t;

#define ptr_array_index(pta, n) (pta)->data[n]
#define uint_array_index(uta, n) (uta)->data[n]

ptr_array_t* ptr_array_new();
void ptr_array_add(ptr_array_t *pta, void *ptr);
void ptr_array_clear(ptr_array_t *pta);
void ptr_array_remove(ptr_array_t *pta, void *ptr);

uint_array_t* uint_array_new();
void uint_array_add(uint_array_t *uta, int n);
void uint_array_clear(uint_array_t *uta);
void uint_array_remove(uint_array_t *uta, uint32_t n);

table_t* table_new();
void table_insert(table_t *tbl, const char *key, void *val);
void* table_lookup(table_t *tbl, const char *key);
void table_clear(table_t *tbl);

#endif
