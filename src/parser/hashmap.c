#include "hashmap.h"
#include <stdlib.h>
#include <string.h>

static uint64_t hash_str(const char *str) {
    // FNV-1a 64-bit
    uint64_t hash = 1469598103934665603ULL;
    while (*str) {
        hash ^= (unsigned char)(*str++);
        hash *= 1099511628211ULL;
    }
    return hash;
}

static void hashmap_resize(HashMap *map);

HashMap *hashmap_create(size_t initial_capacity) {
    HashMap *map = malloc(sizeof(HashMap));
    map->capacity = initial_capacity;
    map->size = 0;
    map->entries = calloc(initial_capacity, sizeof(HashEntry));
    return map;
}

void hashmap_free(HashMap *map) {
    for (size_t i = 0; i < map->capacity; i++) {
        if (map->entries[i].used) free(map->entries[i].key);
    }
    free(map->entries);
    free(map);
}

static void hashmap_resize(HashMap *map) {
    size_t old_capacity = map->capacity;
    HashEntry *old_entries = map->entries;

    map->capacity *= 2;
    map->entries = calloc(map->capacity, sizeof(HashEntry));
    map->size = 0;

    for (size_t i = 0; i < old_capacity; i++) {
        if (old_entries[i].used) {
            hashmap_insert(map, old_entries[i].key, old_entries[i].value);
            free(old_entries[i].key);
        }
    }

    free(old_entries);
}

bool hashmap_insert(HashMap *map, const char *key, void *value) {
    if (map->size * 2 >= map->capacity) hashmap_resize(map);

    uint64_t hash = hash_str(key);
    size_t index = hash % map->capacity;

    while (map->entries[index].used) {
        if (strcmp(map->entries[index].key, key) == 0) {
            map->entries[index].value = value;
            return true;
        }
        index = (index + 1) % map->capacity;
    }

    map->entries[index].key = strdup(key);
    map->entries[index].value = value;
    map->entries[index].used = true;
    map->size++;
    return true;
}

void *hashmap_get(HashMap *map, const char *key) {
    uint64_t hash = hash_str(key);
    size_t index = hash % map->capacity;

    while (map->entries[index].used) {
        if (strcmp(map->entries[index].key, key) == 0)
            return map->entries[index].value;
        index = (index + 1) % map->capacity;
    }
    return NULL;
}

bool hashmap_remove(HashMap *map, const char *key) {
    uint64_t hash = hash_str(key);
    size_t index = hash % map->capacity;

    while (map->entries[index].used) {
        if (strcmp(map->entries[index].key, key) == 0) {
            free(map->entries[index].key);
            map->entries[index].key = NULL;
            map->entries[index].value = NULL;
            map->entries[index].used = false;
            map->size--;
            return true;
        }
        index = (index + 1) % map->capacity;
    }
    return false;
}
