/* 8c01614c */
#ifndef _01614C_REPLAY_MENU_H
#define _01614C_REPLAY_MENU_H

#include "014a9c_tasks.h"
#include "014b8c_backup.h"

void ReplayMenuOpen_8c01673a(void);
void ReplayMenuDemoRecordTask_8c01677e(Task *task, void *state);

typedef struct {
    int courseId_0x00;
    int startStopIndex_0x04;
    int driveMode_0x08;
} ReplayMenuCourseSel;

typedef struct {
    Uint32 on;   /* 0x00 */
    Sint8  x1;   /* 0x04 */
    Uint8  r;    /* 0x05 */
    Uint8  l;    /* 0x06 */
    Uint8  pad;  /* 0x07 */
} ReplayInput;

#define REPLAY_BUFFER_CAPACITY 54000

extern ReplayMenuCourseSel *var_replayMenuCourseSel_8c1bc824;
extern ReplayInput var_demoBuffer_8c1bc828[REPLAY_BUFFER_CAPACITY];

/* The VISUAL_MEMORY entry's VMU picker: it only records which drive a later
 * replay save should use. */
typedef enum {
    REPLAY_SAVE_MENU_INIT = 0,        /* 11-frame settle, then scan for usable drives */
    REPLAY_SAVE_MENU_SELECT = 1,
    REPLAY_SAVE_MENU_CHECK = 2,       /* wait for card ready, validate format/space */
    REPLAY_SAVE_MENU_CONFIRM = 3,     /* free/total blocks; A commits the drive */
    REPLAY_SAVE_MENU_NO_SAVING = 4,   /* "NO SAVING OK?" confirm */
    REPLAY_SAVE_MENU_UNFORMATTED = 5,
    REPLAY_SAVE_MENU_NO_SPACE = 6,
    REPLAY_SAVE_MENU_NO_VMU = 7,      /* no drive can take the save */
    REPLAY_SAVE_MENU_EXIT = 8
} ReplaySaveMenuStateId;

typedef struct {
    ReplaySaveMenuStateId state_0x00;
    int selectedVmu_0x04;           /* cursor over slots 0-7 plus the NO SAVING row (8) */
    int port_0x08;                  /* drive chosen with A */
    int frameCounter_0x0c;
    const BACKUPINFO *bupInfo_0x10;
} SaveMenuState;

void ReplayMenuKillDriveTasks_8c01614c(void);
void ReplayMenuFreeSessionAssets_8c016182(void);
void ReplayMenuResetDemoCursor_8c016770(void);
void ReplayMenuDemoPlayTask_8c016bf4();

#endif // _01614C_REPLAY_MENU_H
