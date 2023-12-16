#ifndef DYNAMICALLY_SCALED_ARRAY_H_
#define DYNAMICALLY_SCALED_ARRAY_H_


#include <stdlib.h>

#define DSA_INITIAL_ELEMENT_COUNT 2
#define DSA_INDEX_TO_P(dsa, index) (((char*)(dsa->data)) + ((dsa->elementSize) * (index))) 
#define DSA_INDEX_AS(dsa, index, type) (((type*)(dsa->data))[index])

typedef struct DSA {
    void *data;
    size_t length;
    size_t allocatedSize;
    size_t elementSize;
}DSA;

DSA *dsa_create(size_t elementSize, size_t initialElementCount);
int dsa_add(DSA *dsa, const void *element);
int dsa_remove(DSA *dsa, size_t index);
int dsa_insert(DSA *dsa, size_t index, const void *element);
int dsa_replace(DSA *dsa, size_t index, const void *element);
int dsa_clear(DSA *dsa);
int dsa_add_multiple(DSA *dsa, const void *arr, size_t arrSize);
int dsa_remove_multiple(DSA *dsa, const size_t *indicies, size_t indiciesLength);
void dsa_free(DSA *dsa);
int dsa_shrink2_used_size(DSA *dsa);
int dsa_allocate_additional(DSA *dsa, size_t numberOfElements);
int dsa_find(DSA *dsa, const void *element, size_t *indexBuf);

#endif