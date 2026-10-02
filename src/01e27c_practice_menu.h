#ifndef _01E27C_H
#define _01E27C_H

#include "014a9c_tasks.h"

/* Selected PRACTICE lesson, 0-10: indexes init_practiceRules_8c0451c0, the
 * description-page table, and var_progress_8c1ba1cc.practiceLessonBestScores_0x98. */
extern int var_practiceLesson_8c22640c;
/* Which parts of a normal run still apply to the selected practice drill,
 * from init_practiceRules_8c0451c0. A set bit keeps the normal
 * behaviour; a clear one takes the drill shortcut, and every reader pairs it
 * with var_playMode_8c1bb8d0 == PLAY_MODE_PRACTICE. 1 = stop announcements and
 * stop-arrival grading (020214, 02b464), 2 = the schedule (02c884, 02b464's
 * INSTR_TIME_MANAGEMENT), 4 = the stop sequence and run completion (02b464),
 * 8 = passengers and bus stops at all (02d968, 02d19c, 022464, and 023310's
 * blinker/mirror start). */
extern int var_practiceRules_8c226410;

void PracticeMenuEnter_8c01f114(Task *task);
void PracticeMenuReturn_8c01f21c(void);

#endif // _01E27C_H
