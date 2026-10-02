/* 8c0129cc */
#ifndef _GAME_H
#define _GAME_H

#include <njdef.h>
#include "014a9c_tasks.h"

#define TEX_NUM 3072

extern NJS_TEXMEMLIST var_tex_8c157af8[TEX_NUM];

extern NJS_TEXLIST init_renderTexlist_8c03bf44;

void GameSoftResetTask_8c012f44();
/* Set up the driving scene; run at course entry and at every segment
 * boundary, from 013ae8's load tasks. */
void GameEnterDrive_8c01306e(void);
/* Start a run on the course already staged in var_replayMenuCourseSel_8c1bc824
 * (debug menu, demos, replays). GameStartCourse_8c013310 is the retail
 * path, taking the course id and the drive mode from saved progress instead. */
void GameStartSelectedCourse_8c01328c();
void GameStartCourse_8c013310(int courseId);
void GameInit_8c0134ec();
int GameMain_8c01392e(void);
void GameExit_8c0139d4(void);

#endif // _GAME_H
