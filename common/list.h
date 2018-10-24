#pragma once

#include <stdlib.h>

/*
 * this structure defines the elements of list
 */
typedef struct list {
    int length;
    size_t arrayLength;
    size_t dataSize;
    void ** values;
} * List;

/**
 * this function creates a list of the given length
 */
List createList(size_t startingLength, size_t dataSize);

/**
 * this function gets the value at the given index
 */
void * getValueAt(List list, int index);

/**
 * this function adds a value to the list
 */
void add(List list, void * value);

/**
 * this function removes a value from the list by the index.
 */
void removeAt(List list, int index);

/*
 * this function releases the memory captured
 */
void freeList(List list);
