/* 8c016d2c - Course Menu */
#ifndef _COURSE_MENU_H
#define _COURSE_MENU_H

#include <shinobi.h>
#include "014a9c_tasks.h"
#include "015ab8_title.h"

int CourseMenuInterpolateCursor_8c016d2c(void);
void CourseMenuRequestCommonResources_8c01852c(void);
int CourseMenuRequestSysResgrp_8c018568(ResourceGroup *res_group, ResourceGroupInfo *res_group_info);
void CourseMenuFreeResourceGroup_8c0185c4(ResourceGroup *res_group);
void CourseMenuStoryMenuTask_8c017718(Task *task, void *state);
void CourseMenuFreeRunMenuTask_8c017ada(Task *task, void *state);
void CourseMenuSwitchFromTask_8c017e18(Task *task);
void CourseMenuDrawDateAndExp_8c016ee6(void);
void CourseMenuPushDialogTask_8c0170c6(int dialog_index, int *p2);
int CourseMenuBuildCourseUnlockList_8c0172dc(void);
void CourseMenuApplyUnlocks_8c0173e6(void);
void CourseMenuFUN_8c017ef2(void);
void CourseMenuConfirmInit_8c0184cc(Task *task);

/* =========================================
 * Future Improvements
 * =========================================
 *
 * 2. Decouple types: Move ResourceGroup/ResourceGroupInfo to a dedicated header
 * 3. Improve naming: Replace FUN_* names with descriptive identifiers
 * 4. Const-correctness: Add const qualifiers where appropriate
 */

#endif /* _COURSE_MENU_H */

