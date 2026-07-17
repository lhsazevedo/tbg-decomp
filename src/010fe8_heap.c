/* @unit Heap */
#include <shinobi.h>
#include "010fe8_heap.h"

#define HEAP_FLAG_USED  1

typedef struct HeapChunk {
    struct HeapChunk* prev_0x00;
    struct HeapChunk* next_0x04;
    int units_0x08;
    int flags_0x0c;
    int reserved_0x10;
    int reserved_0x14;
    int reserved_0x18;
    int reserved_0x1c;
} HeapChunk;

HeapChunk *var_heapHead_8c157a58;
HeapChunk *var_heapTail_8c157a5c;

#ifdef UNIT_TESTING
void heapAllocOutOfMemoryHook_8c01102a(void) {}
#endif

void HeapInit_8c010fe8(void *start, void *end)
{
    HeapChunk *head = (HeapChunk *)start;
    HeapChunk *tail = (HeapChunk *)((char *)end - sizeof(HeapChunk));

    var_heapHead_8c157a58 = head;
    head->flags_0x0c = 0;
    head->units_0x08 = ((char *)tail - (char *)head) / sizeof(HeapChunk);
    head->prev_0x00 = head;
    head->next_0x04 = tail;

    var_heapTail_8c157a5c = tail;
    tail->flags_0x0c = 0;
    tail->units_0x08 = 0;
    tail->prev_0x00 = head;
    tail->next_0x04 = NULL;
}

void *HeapAlloc_8c01102a(int size)
{
    int neededUnits;
    HeapChunk *chunk;
    HeapChunk *oldNext;
    HeapChunk *remainder;

    neededUnits = ((unsigned int)(size + 0x1f) >> 5) + 1;

    chunk = var_heapHead_8c157a58;
    while (1) {
        if (chunk == var_heapTail_8c157a5c) {
#ifdef UNIT_TESTING
            heapAllocOutOfMemoryHook_8c01102a();
#endif
            while (1) {
            }
        }

        if (((chunk->flags_0x0c & HEAP_FLAG_USED) == 0) &&
            (neededUnits <= chunk->units_0x08)) {
            break;
        }

        chunk = chunk->next_0x04;
    }

    oldNext = chunk->next_0x04;

    chunk->units_0x08 = neededUnits;
    chunk->next_0x04 = (HeapChunk *)((char *)chunk + neededUnits * sizeof(HeapChunk));
    chunk->flags_0x0c = HEAP_FLAG_USED;

    remainder = chunk->next_0x04;

    if (remainder != oldNext) {
        remainder->prev_0x00 = chunk;
        remainder->next_0x04 = oldNext;
        remainder->units_0x08 = ((int)oldNext - (int)remainder) / sizeof(HeapChunk);
        remainder->flags_0x0c = 0;

        oldNext->prev_0x00 = remainder;

        if ((oldNext != var_heapTail_8c157a5c) &&
            ((oldNext->flags_0x0c & HEAP_FLAG_USED) == 0)) {
            remainder->next_0x04 = oldNext->next_0x04;
            remainder->units_0x08 += oldNext->units_0x08;
            remainder->next_0x04->prev_0x00 = remainder;
        }
    }

    return (void *)((char *)chunk + sizeof(HeapChunk));
}

int HeapFree_8c0110c4(void *ptr)
{
    HeapChunk *chunk;
    HeapChunk *next;
    HeapChunk *prev;

    chunk = (HeapChunk *)((char *)ptr - sizeof(HeapChunk));

    chunk->flags_0x0c &= ~1;

    next = chunk->next_0x04;
    if ((next != var_heapTail_8c157a5c) && ((next->flags_0x0c & 1) == 0)) {
        chunk->next_0x04 = next->next_0x04;
        chunk->units_0x08 += next->units_0x08;
        chunk->next_0x04->prev_0x00 = chunk;
    }

    prev = chunk->prev_0x00;
    if ((chunk != var_heapHead_8c157a58) && ((prev->flags_0x0c & 1) == 0)) {
        prev->next_0x04 = chunk->next_0x04;
        prev->units_0x08 += chunk->units_0x08;
        chunk->next_0x04->prev_0x00 = prev;
    }

    return 0;
}
