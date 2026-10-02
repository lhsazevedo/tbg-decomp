#ifndef _01E27C_H
#define _01E27C_H

#include "014a9c_tasks.h"

/* 0-10 */
extern int var_practiceLesson_8c22640c;
/* Which parts of a normal run the practice lesson keeps; a clear bit takes
 * the drill shortcut, and readers also check var_playMode_8c1bb8d0 ==
 * PLAY_MODE_PRACTICE. 1 = stop announcements and stop-arrival grading,
 * 2 = the schedule, 4 = the stop sequence and run completion, 8 = passengers
 * and bus stops at all. */
extern int var_practiceRules_8c226410;

void PracticeMenuEnter_8c01f114(Task *task);
void PracticeMenuReturn_8c01f21c(void);

#endif // _01E27C_H
