#include <stdlib.h>
#include <string.h>

#include "datatypes.h"

ptr_array_t* ptr_array_new() {
    ptr_array_t *pta = malloc(sizeof(ptr_array_t));
    pta->len = 0;
    pta->data = NULL;
    return pta;
}

void ptr_array_add(ptr_array_t *pta, void *ptr) {
    if (!pta->len)
        pta->data = malloc(sizeof(void *));
    else
        pta->data = realloc(pta->data, sizeof(void *) * (pta->len + 1));
    
    pta->data[pta->len++] = ptr;
}

void ptr_array_clear(ptr_array_t *pta) {
    free(pta->data);
    pta->data = NULL;
    pta->len = 0;
}

void ptr_array_remove(ptr_array_t *pta, void *ptr) {
    for (uint i = 0; i < pta->len; i++) {
        if (ptr_array_index(pta, i) == ptr) {
            for (uint j = i; j < pta->len; i++)
                pta->data[j] = pta->data[j + 1];
            pta->len--;
            break;
        }
    }
}

uint_array_t* uint_array_new() {
    uint_array_t *uta = malloc(sizeof(uint_array_t));
    uta->data = NULL;
    uta->len = 0;
    return uta;
}

void uint_array_add(uint_array_t *uta, int n) {
    if (!uta->len)
        uta->data = malloc(sizeof(uint32_t));
    else
        uta->data = realloc(uta->data, sizeof(uint32_t) * (uta->len + 1));
    
    uta->data[uta->len++] = n;
}

void uint_array_clear(uint_array_t *uta) {
    free(uta->data);
    uta->data = NULL;
    uta->len = 0;
}

void uint_array_remove(uint_array_t *uta, uint32_t n) {
    for (uint i = 0; i < uta->len; i++) {
        if (uint_array_index(uta, i) == n) {
            for (uint j = i; j < uta->len; i++)
                uta->data[j] = uta->data[j + 1];
            uta->len--;
            break;
        }
    }
}

table_t* table_new() {
    table_t *tbl = malloc(sizeof(table_t));
    tbl->keys = NULL;
    tbl->vals = NULL;
    tbl->len = 0;
    return tbl;
}

void table_insert(table_t *tbl, const char *key, void *val) {
    if (!tbl->len) {
        tbl->keys = malloc(sizeof(char *));
        tbl->vals = malloc(sizeof(void *));
    } else {
        tbl->keys = realloc(tbl->keys, sizeof(char *) * (tbl->len + 1));
        tbl->vals = realloc(tbl->vals, sizeof(void *) * (tbl->len + 1));
    }
    tbl->keys[tbl->len] = strdup(key);
    tbl->vals[tbl->len] = val;
    tbl->len++;
}

void* table_lookup(table_t *tbl, const char *key) {
    for (uint i = 0; i < tbl->len; i++) {
        if (!strcmp(tbl->keys[i], key))
            return tbl->vals[i];
    }
    return NULL;
}

void table_clear(table_t *tbl) {
    for (uint i = 0; i < tbl->len; i++)
        free(tbl->keys[i]);
    free(tbl->keys);
    free(tbl->vals);
    tbl->keys = NULL;
    tbl->vals = NULL;
    tbl->len = 0;
}
