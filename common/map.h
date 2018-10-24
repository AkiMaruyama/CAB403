#pragma once

#include <stdbool.h>
#include "list.h"

/*
 * this structure defines elements of map entry
 */
typedef struct map_entry {
    char * key;
    void * value;
} * MapEntry;

/*
 * this structure defines elements of map
 */
typedef struct map {
    int length;
    MapEntry * entries;
} * Map;

/**
 * this function creates map of the given length
 */
Map createMap(int startingLength);

/**
 * this function gets a value from map, or NULL
 */
void * getValue(Map map, char * key);

/**
 * this function puts an entry in the map
 */
void putEntry(Map map, char * key, void * value);

/**
 * this function removes an entry from the map
 */
bool removeEntry(Map map, char * key);

/**
 * this function gets if map has a key
 */
bool containsEntry(Map map, char * key);

/*
 * this function gets the map values
 */
void ** getValues(Map map, size_t size, int * length);

/*
 * this function releases memory captured
 */
void freeMap(Map map);
