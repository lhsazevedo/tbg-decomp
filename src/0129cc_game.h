/* 8c0129cc */
#ifndef _GAME_H
#define _GAME_H

#include <njdef.h>
#include "014a9c_tasks.h"

#define TEX_NUM 3072

extern NJS_TEXMEMLIST var_tex_8c157af8[TEX_NUM];

extern NJS_TEXLIST init_renderTexlist_8c03bf44;

void GameSoftResetTask_8c012f44();
/* Runs at course entry and at every segment boundary. */
void GameEnterDrive_8c01306e(void);
/* Starts a run on the course staged in var_replayMenuCourseSel_8c1bc824
 * (debug menu, demos, replays). */
void GameStartSelectedCourse_8c01328c();
/* Drive mode comes from saved progress. */
void GameStartCourse_8c013310(int courseId);
void GameInit_8c0134ec();
int GameMain_8c01392e(void);
void GameExit_8c0139d4(void);

#endif // _GAME_H
