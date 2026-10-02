/* @unit ResGroup */
/* 8c016108 */
#include <shinobi.h>
#include "015ab8_title.h"
#include "016d2c_course_menu.h"
#include "016108_resgrp_free.h"
#include "01c980_profile_file.h" /* var_resourceGroup_8c2263a8 */
#include "02a9fc_message_box.h" /* MessageBoxFreeTextboxes_8c02af32 */

/* ====================
 * Functions
 * ====================
 */

void ResGroupFreeAll_8c016108()
{
    CourseMenuFreeResourceGroup_8c0185c4(&var_menuState_8c1bc7a8.resourceGroupA_0x00);
    CourseMenuFreeResourceGroup_8c0185c4(&var_menuState_8c1bc7a8.resourceGroupB_0x0c);
    CourseMenuFreeResourceGroup_8c0185c4(&var_resourceGroup_8c2263a8);

    MessageBoxFreeTextboxes_8c02af32();
    /* No \SYSTEM group resident: the next request reloads it. */
    var_currentSysResGroupInfo_8c225fb0 = (void *) -1;
}
