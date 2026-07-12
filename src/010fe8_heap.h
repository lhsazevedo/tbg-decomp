/* 8c010fe8 */
#ifndef _HEAP_H
#define _HEAP_H

void heapInit_8c010fe8(void *start, void *end);
void *heapAlloc_8c01102a(int size);
int heapFree_8c0110c4(void *ptr);

#endif // _HEAP_H
