/* 8c014a9c */
#ifndef _014A9C_TASKS_H
#define _014A9C_TASKS_H

#include <shinobi.h>

/* =================
 * Type Declarations
 * =================
 */

typedef void (*TaskAction)(struct Task *task, void *state);

/* A group is an array of these ended by a NULL action; a -1 action marks a
 * free slot. Only action and state belong to the dispatcher, the rest is
 * overlaid by each task type. */
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

int TaskSpawn_8c014ae8(Task *tasks, void *action, Task **created_task, void **create_state, size_t alloc_size);
void TaskKill_8c014b66(Task *task);
void TaskInitGroup_8c014a9c(Task *tasks, Sint32 count);
void TaskKillGroup_8c014ab4(Task *tasks);
void TaskSwitch_8c014b3e(Task *task, TaskAction action);
void TaskRunGroup_8c014b42(Task task[]);

#endif // _014A9C_TASKS_H
