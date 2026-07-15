/* 8c0129cc - Pause Menu */
#ifndef _PAUSE_H
#define _PAUSE_H

#include "014a9c_tasks.h"

/* Task layout as seen by PauseDemoEndTask_8c012d5a. */
typedef struct {
    TaskAction action;
    void *state;
    int phase_0x08;      /* DEMO_END_* */
    int counter_0x0c;    /* frame counter */
    int field_0x10;
    int field_0x14;
    void *queuedItem_0x18;
    int field_0x1c;
} PauseDemoEndTaskData;

void PauseTask_8c012cbc();
void PauseToggleTask_8c012d06();
void PauseDemoEndTask_8c012d5a(PauseDemoEndTaskData *task);

#endif // _PAUSE_H
