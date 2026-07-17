#include "shinobi.h"

#ifndef _TASKS_H_
#define _TASKS_H_

// Probably should be moved to another header...
struct QueuedDat {
    char *basedir;
    char *filename;
    void **dest;
    int field_0x0c;
}
typedef QueuedDat;

typedef void (*TaskAction)(struct Task *task, void *state);

struct Task {
    TaskAction action;
    void *state;
    int field_0x08;
    void* field_0x0c;
    int field_0x10;
    int field_0x14;
    void* queuedItem_0x18;
    int field_0x1c;
}
typedef Task;

/**
 * @todo Should action be typed?
 */
int TaskPush_8c014ae8(Task *tasks, void *action, Task **created_task, void **create_state, size_t alloc_size);
void TaskFree_8c014b66(Task *task);
void TaskClear_8c014a9c(Task *tasks, Sint32 count);
void TaskFreeGroup_8c014ab4(Task *tasks);
void TaskSetAction_8c014b3e(Task *task, TaskAction action);
void TaskExecGroup_8c014b42(Task task[]);

#endif /* _TASKS_H_ */
