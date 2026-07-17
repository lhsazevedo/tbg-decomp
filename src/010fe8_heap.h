/* 8c010fe8 */
#ifndef _HEAP_H
#define _HEAP_H

void HeapInit_8c010fe8(void *start, void *end);
void *HeapAlloc_8c01102a(int size);
int HeapFree_8c0110c4(void *ptr);

#endif // _HEAP_H
