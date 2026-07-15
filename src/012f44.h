/* 8c012f44 */
#ifndef _012F44_H
#define _012F44_H

#include <njdef.h>
#include "014a9c_tasks.h"

#define TEX_NUM 3072

extern NJS_TEXMEMLIST var_tex_8c157af8[TEX_NUM];

extern int var_pauseSettle_8c18ad04;
extern int var_retirePhase_8c18ad08;
extern int var_confirmChoice_8c18ad0c;
extern int var_onRetire_8c18ad10;
extern char init_8c03bf4c[];

void task_8c012f44();
void FUN_8c01306e(void);
void FUN_8c01328c();
void pushLoadingTask_8c013310(int p1);
void njUserInit_8c0134ec();
int njUserMain_8c01392e(void);
void njUserExit_8c0139d4(void);

#endif // _012F44_H
