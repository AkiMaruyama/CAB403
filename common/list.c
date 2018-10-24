#include "list.h"
#include <memory.h>

/*
 * this function creates a list of pointed length
 */
List createList(size_t startingLength, size_t dataSize) {
    List list = calloc(1, sizeof(struct list));
    list->length = 0;
    list->arrayLength = startingLength;
    list->dataSize = dataSize;
    list->values = calloc(list->arrayLength, dataSize);
    return list;
}

/*
 * this function gets value by the index
 */
void * getValueAt(List list, int index) {
    if (index < 0 || index > list->length - 1) {
        return NULL;
    }

    return list->values[index];
}

/*
 * this function adds a value
 */
void add(List list, void * value) {
    list->length++;
    if (list->length > list->arrayLength) {
        size_t oldLength = list->arrayLength;
        list->arrayLength <<= 1;
        list->values = realloc(list->values, list->arrayLength * list->dataSize);

        for (size_t i = oldLength; i < list->arrayLength; i++) {
            list->values[i] = NULL;
        }
    }

    list->values[list->length - 1] = value;
}

/*
 * this function removes the value by the index
 */
void removeAt(List list, int index) {
    if (index < 0 || index > list->length - 1) {
        return;
    }

    memmove(list->values + index, list->values + (index + 1),
            (size_t) (list->arrayLength - index) * sizeof(*(list->values)));
    list->length--;
}

/*
 * this function releases memory captured
 */
void freeList(List list) {
    free(list->values);
    free(list);
}
