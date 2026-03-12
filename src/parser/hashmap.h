#ifndef HASHMAP_H
#define HASHMAP_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define _POSIX_C_SOURCE 200809L
#include <string.h>

// A simple open-addressed hash map with linear probing.

typedef struct {
    char *key;
    void *value;
    bool used;
} HashEntry;

typedef struct {
    HashEntry *entries;
    size_t capacity;
    size_t size;
} HashMap;

HashMap *hashmap_create(size_t initial_capacity);
void hashmap_free(HashMap *map);
bool hashmap_insert(HashMap *map, const char *key, void *value);
void *hashmap_get(HashMap *map, const char *key);
bool hashmap_remove(HashMap *map, const char *key);

#endif // HASHMAP_H
