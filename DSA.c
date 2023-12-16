#include <stdlib.h>
#include <stdio.h>
#include "DSA.h"

#define DSA_SIZE_COEFFICIENT 2
#define DSA_USED_SIZE(dsa) ((dsa->length) * (dsa->elementSize))
#define DSA_LAST_INDEX(dsa) (dsa->length - 1)
#define DSA_ERR_INVALID_INDEX "Invalid index."


int dsa_size_t_compare(const void *a, const void *b)
{
    const size_t _a = *(const size_t *)a;
    const size_t _b = *(const size_t *)b;
    if (_a < _b)
    {
        return -1;
    }
    if (_a > _b)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void dsa_memcpy(void *dest, const void *src, size_t size)
{
    char *d = dest;
    const char *s = src;
    while (size--)
    {
        *d++ = *s++;
    }
}

void dsa_memcpy_reverse(void *destEnd, const void *srcEnd, size_t size)
{
    char *d = destEnd;
    const char *s = srcEnd;
    while (size--)
    {
        *d-- = *s--;
    }
}

int dsa_add_size_handle(DSA *dsa, size_t addSize)
{
    if (dsa->allocatedSize >= DSA_USED_SIZE(dsa)+addSize)
    {
        return 1;
    }
    size_t newSize = DSA_SIZE_COEFFICIENT * (DSA_USED_SIZE(dsa) + addSize);
    void *temp = realloc(dsa->data, newSize);
    if (temp == NULL)
    {
        perror("dsa_add_size_handle: ");
        return 0;
    }
    dsa->data = temp;
    dsa->allocatedSize = newSize;
    return 1;
}

void dsa_shift_block_right(DSA *dsa, size_t blockStartIndex, size_t blockEndIndex, size_t shiftCount)
{
    char *srcEnd = DSA_INDEX_TO_P(dsa, blockEndIndex) + (dsa->elementSize - 1);
    char *destEnd = srcEnd + (dsa->elementSize * shiftCount);
    size_t shiftedElementCount = blockEndIndex - blockStartIndex + 1;
    dsa_memcpy_reverse(destEnd, srcEnd, shiftedElementCount * dsa->elementSize);
}

void dsa_shift_block_left(DSA *dsa, size_t blockStartIndex, size_t blockEndIndex, size_t shiftCount)
{
    char *src = DSA_INDEX_TO_P(dsa, blockStartIndex);
    char *dest = src - (dsa->elementSize * shiftCount);
    size_t shiftedElementCount = blockEndIndex - blockStartIndex + 1;
    dsa_memcpy(dest, src, shiftedElementCount * dsa->elementSize);
}

size_t dsa_remove_multi_shift_handle(DSA *dsa, size_t *indiciesSorted, size_t length)
{
    size_t i = 0;
    size_t totalShiftCount = 0;
    size_t shiftCount = 1;
    do
    {
        if (indiciesSorted[i] == dsa->length - 1)
        {
            totalShiftCount += shiftCount;
            shiftCount = 1;
            break;
        }
        if (i == length - 1)
        {
            totalShiftCount += shiftCount;
            shiftCount = 1;
            dsa_shift_block_left(dsa, indiciesSorted[i] + 1, dsa->length - 1, totalShiftCount); 
        }
        else
        {
            if (indiciesSorted[i+1] == indiciesSorted[i])
            {
                continue;
            }
            else if (indiciesSorted[i+1] - indiciesSorted[i] == 1)
            {
                shiftCount++;
            }
            else
            {
                totalShiftCount += shiftCount;
                shiftCount = 1;
                dsa_shift_block_left(dsa, indiciesSorted[i] + 1, indiciesSorted[i+1] - 1, totalShiftCount);
            }
        }
    } while (++i < length);
    return totalShiftCount;
}


DSA *dsa_create(size_t elementSize, size_t initialElementCount)
{
    if (elementSize == 0 && initialElementCount == 0){
        printf("dsa_create: element size or initial element count can not be zero\n");
        return NULL;
    }
    DSA *dsa = (DSA*)malloc(sizeof(DSA));
    if (!dsa){
        perror("dsa_create:");
        return NULL;
    }
    dsa->elementSize = elementSize;
    dsa->allocatedSize = dsa->elementSize * initialElementCount;
    dsa->length = 0;
    dsa->data = malloc(dsa->allocatedSize);
    if (!dsa->data)
    {
        free(dsa);
        perror("dsa_create: ");
        return NULL;
    }
    return dsa;
}

int dsa_add(DSA *dsa, const void *element)
{
    /* Null check */
    if (!dsa || !element)
    {
        return 0;
    }

    // Handle size
    if (!dsa_add_size_handle(dsa, dsa->elementSize))
    {
        return 0;
    }
    char *p = DSA_INDEX_TO_P(dsa, dsa->length);
    dsa_memcpy(p, element, dsa->elementSize);
    dsa->length++;
    return 1;
}

int dsa_remove(DSA *dsa, size_t index)
{
    /* Null check */
    if (!dsa)
    {
        return 0;
    }

    /* Check index is valid */ 
    if (index >= dsa->length)
    {
        printf(DSA_ERR_INVALID_INDEX);
        return 0;
    }

    if (index != dsa->length-1)
    {
        dsa_shift_block_left(dsa, index+1, dsa->length-1, 1);
    }
    dsa->length--;
    return 1;
}

int dsa_insert(DSA *dsa, size_t index, const void *element)
{
    /* Null check */
    if (!dsa || !element)
    {
        return 0;
    }

    if (index >= dsa->length  || dsa->length == 0)
    {
        return dsa_add(dsa, element);
    }
    
    if (!dsa_add_size_handle(dsa, dsa->elementSize))
    {
        return 0;
    }
    
    dsa_shift_block_right(dsa, index, dsa->length-1, 1);
    char *p = DSA_INDEX_TO_P(dsa, index);
    dsa_memcpy(p, element, dsa->elementSize);
    dsa->length++;
    return 1;
}

int dsa_clear(DSA *dsa)
{
    /* Null check */
    if (!dsa)
    {
        return 0;
    }
    dsa->length = 0;
    return 1;
}

int dsa_add_multiple(DSA *dsa, const void *arr, size_t arrLength)
{
    if (!dsa || !arr || !arrLength)
    {
        return 0;
    }
    
    if (!dsa_add_size_handle(dsa, arrLength*dsa->elementSize))
    {
        printf("dsa_add: size could not handled.");
        return 0;
    }
    char *p = DSA_INDEX_TO_P(dsa, dsa->length);
    dsa_memcpy(p, arr, arrLength*dsa->elementSize);
    dsa->length+=arrLength;
    return 1;
}

int dsa_remove_multiple(DSA *dsa, const size_t *indicies, size_t indiciesLength)
{
    if (!dsa || !indicies || !indiciesLength)
    {
        return 0;
    }
    if (dsa->length == 0)
    {
        return 1;
    }
    if (indiciesLength == 1)
    {
        return dsa_remove(dsa, indicies[0]);
    }
    size_t *indicies_sorted = malloc(sizeof(size_t) * indiciesLength);
    dsa_memcpy(indicies_sorted, indicies, sizeof(size_t) * indiciesLength);
    qsort(indicies_sorted, indiciesLength, sizeof(size_t), dsa_size_t_compare);

    if (indicies_sorted[0] >= dsa->length || indicies_sorted[indiciesLength-1] >= dsa->length)
    {
        return 0;
    }
    size_t removedElementCount = dsa_remove_multi_shift_handle(dsa, indicies_sorted, indiciesLength);
    dsa->length -= removedElementCount;
    return  1;

}

void dsa_free(DSA *dsa)
{
    if (dsa)
    {
        free(dsa->data);
        free(dsa);
    }
}

int dsa_shrink2_used_size(DSA *dsa)
{
    if (!dsa)
    {
        return 0;
    }
    if (dsa->allocatedSize <= DSA_USED_SIZE(dsa))
    {
        return 1;
    }
    void *temp = realloc(dsa->data, DSA_USED_SIZE(dsa));
    if (temp == NULL)
    {
        perror("dsa_shrink2_used_size: ");
        return 0;
    }
    dsa->data = temp;
    dsa->allocatedSize = DSA_USED_SIZE(dsa);
    return 1;
}

int dsa_allocate_additional(DSA *dsa, size_t numberOfElements)
{
    if (!dsa)
    {
        return 0;
    }
    if (!numberOfElements)
    {
        return 1;
    }
    size_t newSize = dsa->allocatedSize + (dsa->elementSize * numberOfElements);
    void *temp = realloc(dsa->data, newSize);
    if (temp == NULL)
    {
        perror("dsa_allocate_additional: ");
        return 0;
    }
    dsa->data = temp;
    dsa->allocatedSize = newSize;
    return 1;
}

int dsa_find(DSA *dsa, const void *element, size_t *indexBuf)
{ 
    /* Null Check */ 
    if (!dsa)
    {
        return 0;
    }
    const char *elementP = element;
    for (size_t i = 0; i<dsa->length; i++)
    {
        int found = 1;
        for (size_t j = 0; j<dsa->elementSize; j++)
        {
            if (*(DSA_INDEX_TO_P(dsa, i) + j) != *(elementP + j))
            {
                found = 0;
                break;
            } 
        }
        if (found)
        {
            if (!indexBuf)
            {
                *indexBuf = i;
            }
            return 1;
        }
    }
    return 0;
}

int dsa_replace(DSA *dsa, size_t index, const void *element)
{
    /* Null check */
    if (!dsa || !element)
    {
        return 0;
    }
    if (index >= dsa->length)
    {
        return 0;
    }
    dsa_memcpy(DSA_INDEX_TO_P(dsa, index), element, dsa->elementSize);
    return 1;
}

