/* COURSE SELECT screen, its confirm/ROUTE INFO sub-screen, and the
 * instructor dialog machinery every menu shares. */
#ifndef _COURSE_MENU_H
#define _COURSE_MENU_H

#include <shinobi.h>
#include "014a9c_tasks.h"
#include "015ab8_title.h"

typedef struct {
    char *text_0x00;
    int spriteNo_0x04;
} InstructorLine;

/* Indices into init_instructorDialogs_8c044c08. */
enum {
    // --- Story / Training ---
    INSTR_STORY_INTRO           = 0,
    INSTR_SUCCESS_PERFECT          = 1,
    INSTR_SUCCESS_HIGH             = 2,
    INSTR_SUCCESS_NORMAL           = 3,
    INSTR_FAILURE_FINAL            = 4,
    INSTR_FREE_RUN_INTRO        = 5,
    INSTR_STORY_CHOOSE_COURSE   = 6,
    INSTR_GOOD_PRACTICE         = 7,
    INSTR_SUCCESS               = 8,

    // --- Awards / Unlocks ---
    INSTR_AWARD_BADGE_GOLD      = 9,
    INSTR_AWARD_BADGE_SILVER    = 10,
    INSTR_AWARD_BADGE_BRONZE    = 11,
    INSTR_FAILURE_RETRY         = 12,
    INSTR_COURSE_UNLOCKED       = 13,
    INSTR_PASSENGER_LETTER      = 14,
    INSTR_COURSE_LOCKED         = 15,
    INSTR_FORCE_PRACTICE        = 16,
    INSTR_FINAL_DAY             = 17,

    // --- Lesson Mode ---
    INSTR_LESSON_INTRO          = 18,
    INSTR_LESSON_COMPLETE       = 19,
    INSTR_LESSON_NEXT           = 20,
    INSTR_LESSON_RETRY          = 21,
    INSTR_LESSON_TIPS           = 22,
    INSTR_LESSON_WARNING        = 23,
    INSTR_LESSON_CHOOSE         = 24,
    INSTR_SCORE_RECORD          = 25,
    INSTR_LESSON_FINAL_DAY      = 26,
    INSTR_LESSON_PERFECT        = 27,
    INSTR_LESSON_GOOD           = 28,
    INSTR_LESSON_PASS           = 29,
    INSTR_LESSON_FAIL_MINOR     = 30,
    INSTR_LESSON_FAIL_MAJOR     = 31,

    // --- Driving Mistakes / Penalties ---
    // Practice mode only: a lesson closes on the run's worst penalty, mapped
    // from its PENALTY_MSG_* id through init_penaltyMsgSetInstr_8c045208.
    // The ones marked "never raised" are not in that table.
    INSTR_COLLISION_CAR_MINOR   = 32,
    INSTR_COLLISION_CAR_MEDIUM  = 33,
    INSTR_COLLISION_CAR_SEVERE  = 34, // never raised
    INSTR_COLLISION_CAR_FATAL   = 35,
    INSTR_COLLISION_WALL_MINOR  = 36,
    INSTR_COLLISION_WALL_MEDIUM = 37,
    INSTR_COLLISION_WALL_SEVERE = 38,
    INSTR_NEAR_MISS_PEDESTRIAN  = 39,
    INSTR_OFF_COURSE_MINOR      = 40,
    INSTR_OFF_COURSE_MEDIUM     = 41,
    INSTR_OFF_COURSE_MAJOR      = 42, // never raised
    INSTR_SPEEDING_MINOR        = 43,
    INSTR_SPEEDING_MAJOR        = 44,
    INSTR_WRONG_LANE            = 45,
    INSTR_LANE_STRADDLE         = 46,
    INSTR_NO_SIGNAL             = 47,
    INSTR_NO_SIGNAL_TURN        = 48, // never raised
    INSTR_BLANK                 = 49, // empty dialog sequence
    INSTR_SIGNAL_VIOLATION      = 50,
    INSTR_BAD_STOP_LINE         = 51,
    INSTR_ILLEGAL_LANE_CHANGE   = 52,
    INSTR_BLOCK_INTERSECTION    = 53,
    INSTR_WRONG_WAY             = 54,
    INSTR_RAPID_ACCEL           = 55,
    INSTR_HARD_BRAKE            = 56,
    INSTR_SWERVING              = 57,
    INSTR_MISSED_STOP           = 58,
    INSTR_BAD_STOP_POSITION_1   = 59,
    INSTR_BAD_STOP_POSITION_2   = 60, // never raised
    INSTR_TIME_MANAGEMENT       = 61,
    INSTR_ANNOUNCEMENT          = 62,
    INSTR_DOOR_OPERATION        = 63,

    // --- Free Run Mode ---
    INSTR_FREE_RUN_INTRO_2       = 64,
    INSTR_FREE_RUN_CHOOSE_COURSE = 65,
};

extern InstructorLine *init_instructorDialogs_8c044c08[66];
extern void* var_currentSysResGroupInfo_8c225fb0;
extern int var_instructorDialogActive_8c225fb4;
extern int var_menuTextboxCharLimit_8c225fb8;
extern int var_dialogQueue_8c225fbc[6];

int CourseMenuInterpolateCursor_8c016d2c(void);
void CourseMenuRequestCommonResources_8c01852c(void);
int CourseMenuRequestSysResgrp_8c018568(ResourceGroup *res_group, ResourceGroupInfo *res_group_info);
void CourseMenuFreeResourceGroup_8c0185c4(ResourceGroup *res_group);
void CourseMenuEnter_8c017e18(Task *task);
void CourseMenuDrawDateAndExp_8c016ee6(void);
void CourseMenuSpawnDialogTask_8c0170c6(int dialog_index, int *voice_cues);
int CourseMenuBuildCourseUnlockList_8c0172dc(void);
void CourseMenuApplyUnlocks_8c0173e6(void);
void CourseMenuReturn_8c017ef2(void);

#endif /* _COURSE_MENU_H */

