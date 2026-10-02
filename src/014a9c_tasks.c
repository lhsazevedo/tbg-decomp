/* @unit Task */
/* 8c014a9c */
#include "shinobi.h"
#include "014a9c_tasks.h"

void TaskInitGroup_8c014a9c(Task *tasks, Sint32 count)
{
  Sint32 i;
  for (i = 0; i < count; ++i)
  {
    tasks->action = (TaskAction) -1;
    tasks++;
  }

  tasks->action = 0;
  return;
}

void TaskUninstallGroup_8c014ab4(Task *tasks)
{
  for (; tasks->action != NULL; tasks++) {
    if (tasks->action != (TaskAction) -1) {
      if (tasks->state != NULL) {
        syFree(tasks->state);
      }
      tasks->action = (void *) -1;
    }
  }
}

int TaskInstall_8c014ae8(Task *tasks, void *action, Task **installed_task, void **installed_state, size_t alloc_size)
{
  void *state;

  for (; tasks->action != (TaskAction) -1; tasks++) {
    if (tasks->action == NULL) {
      return 0;
    }
  }

  if (alloc_size) {
    if ((tasks->state = syMalloc(alloc_size)) == NULL)
      return 0;
  }
  else {
    tasks->state = NULL;
  }

  tasks->action = action;
  *installed_task = tasks;
  *installed_state = tasks->state;

  return 1;
}

void TaskReinstall_8c014b3e(Task *task, TaskAction action)
{
  task->action = action;
}

void TaskRunGroup_8c014b42(Task task[]) {
    for (; task->action != NULL; task++) {
        if (task->action != (TaskAction) -1) {
            task->action(task, task->state);
        }
    }
}

void TaskUninstall_8c014b66(Task *task)
{
  if (task->state != NULL) {
    syFree(task->state);
  }

  task->action = (void *) -1;
  return;
}
