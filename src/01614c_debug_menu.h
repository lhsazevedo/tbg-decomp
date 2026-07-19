/* 8c01614c: undecompiled */
#ifndef _01614C_H
#define _01614C_H

#include "014a9c_tasks.h"
#include "014b8c_backup.h"

void DebugMenuOpen_8c01673a(void);
void DebugMenuDemoRecordTask_8c01677e(Task *task, void *state);

/* course-start params: tail of a DebugMenuEntry, handed off via var_debugMenuCourseSel_8c1bc824 to
 * 012f44_game.c's game-start logic when a debug-menu course entry is picked. */
typedef struct {
    int courseId_0x00;
    int field_0x04;
    int inputMapSel_0x08;
} DebugMenuCourseSel;

/* States of the VMU save/replay menu task saveMenuTask_8c01628c; named by transitions. */
typedef enum {
    DEBUG_SAVE_MENU_INIT = 0,        /* startup spinner, scan for connected VMUs */
    DEBUG_SAVE_MENU_SELECT = 1,      /* choose a VMU slot */
    DEBUG_SAVE_MENU_CHECK = 2,       /* wait for card ready, validate format/space */
    DEBUG_SAVE_MENU_CONFIRM = 3,     /* show capacity, confirm overwrite/new save */
    DEBUG_SAVE_MENU_NO_SAVING = 4,   /* "NO SAVING OK?" confirm */
    DEBUG_SAVE_MENU_UNFORMATTED = 5, /* card is not formatted */
    DEBUG_SAVE_MENU_NO_SPACE = 6,    /* not enough free area */
    DEBUG_SAVE_MENU_NO_VMU = 7,      /* no VMU connected */
    DEBUG_SAVE_MENU_EXIT = 8         /* leave the save flow */
} DebugSaveMenuStateId;

/* WIP state for saveMenuTask_8c01628c (VMU save/replay menu task); fields named by usage. */
typedef struct {
    DebugSaveMenuStateId state_0x00;     /* state-machine state */
    int selectedVmu_0x04;           /* cursor over the connected VMU slots */
    int port_0x08;                  /* confirmed drive/port */
    int frameCounter_0x0c;          /* startup spinner frame count */
    const BACKUPINFO *bupInfo_0x10; /* BupGetInfo result for the selected port */
} SaveMenuState;

void FUN_8c01614c(void);
void FUN_8c016182(void);
void FUN_8c016770(void);

#endif // _01614C_H
