#include <stdlib.h>
#include <string.h>
#include "map.h"

/*
 * this function creates map of given length
 */
Map createMap(int startingLength) {
    Map map = malloc(sizeof(struct map));
    map->length = startingLength;
    map->entries = calloc((size_t) map->length, sizeof(MapEntry));
    return map;
}

/*
 * this function checks if keys are equal
 */
bool areKeysEqual(char * a, char * b) {
    return strcmp(a, b) == 0;
}

/*
 * this function gets value from map
 */
void * getValue(Map map, char * key) {
    for (int i = 0; i < map->length; i++) {
        if (map->entries[i] == NULL) {
            continue;
        }
        if (areKeysEqual(map->entries[i]->key, key)) {
            return map->entries[i]->value;
        }
    }
    return NULL;
}

/*
 * this function gets free index of map
 */
int getFreeIndex(Map map) {
    for (int i = 0; i < map->length; i++) {
        if (map->entries[i] == NULL) {
            return i;
        }
    }
    return -1;
}

void putEntry(Map map, char * key, void * value) {
    if (containsEntry(map, key)) {
        for (int i = 0; i < map->length; i++) {
            if (map->entries[i] == NULL) {
                continue;
            }
            if (map->entries[i]->key == key) {
                map->entries[i]->value = value;
                return;
            }
        }
    } else {
        int freeIndex = getFreeIndex(map);
        if (freeIndex < 0) {
            size_t oldLength = (size_t) map->length;
            map->length <<= 1;
            map->entries = realloc(map->entries, map->length * sizeof(MapEntry));

            for (size_t i = oldLength; i < map->length; i++) {
                map->entries[i] = NULL;
            }

            MapEntry entry = malloc(sizeof(MapEntry));
            entry->key = key;
            entry->value = value;

            map->entries[map->length - 1] = entry;
        } else {
            MapEntry entry = malloc(sizeof(MapEntry));
            entry->key = key;
            entry->value = value;

            map->entries[freeIndex] = entry;
        }
    }
}

/*
 * this function removes entry from map
 */
bool removeEntry(Map map, char * key) {
    for (int i = 0; i < map->length; i++) {
        if (map->entries[i] == NULL) {
            continue;
        }
        if (map->entries[i]->key == key) {
            map->entries[i]->value = NULL;
            return true;
        }
    }
    return false;
}

/*
 * this function checks if map contains entry
 */
bool containsEntry(Map map, char * key) {
    return getValue(map, key) != NULL;
}

/*
 * this function gets values from map
 */
void ** getValues(Map map, size_t size, int * length) {
    void ** values = malloc(map->length * size);
    int index = 0;
    for (int i = 0; i < map->length; i++) {
        if (map->entries[i] == NULL) {
            continue;
        }
        values[index++] = map->entries[i]->value;
    }
    (*length) = index;

    return values;
}

/*
 * this function releases map's memory
 */
void freeMap(Map map) {
    free(map->entries);
    free(map);
}
