#ifndef _PROFILE_FILE_H
#define _PROFILE_FILE_H

#include "014a9c_tasks.h"
#include "015ab8_title.h" /* ResourceGroup */

/* =======================
 * Non-initialized Globals
 * =======================
 */

extern int var_profileUnlockedCount_8c2263a4;
extern ResourceGroup var_resourceGroup_8c2263a8;

/* =========
 * Functions
 * =========
 */

void ProfileFileUpdateUnlocks_8c01c980(void);
void ProfileFileEnter_8c01d1c4(Task *task);

#endif // _PROFILE_FILE_H
