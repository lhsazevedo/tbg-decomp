/* 8c012f44 */
#ifndef _GAME_H
#define _GAME_H

#include <njdef.h>
#include "014a9c_tasks.h"

#define TEX_NUM 3072

extern NJS_TEXMEMLIST var_tex_8c157af8[TEX_NUM];

extern int var_pauseSettle_8c18ad04;
extern int var_retirePhase_8c18ad08;
extern int var_confirmChoice_8c18ad0c;
extern int var_onRetire_8c18ad10;
extern char init_8c03bf4c[];

void GameTask_8c012f44();
void FUN_8c01306e(void);
void FUN_8c01328c();
void GamePushLoadingTask_8c013310(int p1);
void GameInit_8c0134ec();
int GameMain_8c01392e(void);
void GameExit_8c0139d4(void);

#endif // _GAME_H
