/* 8c015ab8: the title screen -- logo sequence, no-VMU warning, PRESS START --
 * and MenuState, the one state block every menu screen takes turns owning. */
#ifndef _TITLE_H_
#define _TITLE_H_

#include <shinobi.h>
#include "014a9c_tasks.h"
#include "014b8c_backup.h"

enum TITLE_STATE {
    TITLE_STATE_0X00_INIT,
    TITLE_STATE_0X01_FORTYFIVE_FADE_IN,
    TITLE_STATE_0X02_FORTYFIVE,
    TITLE_STATE_0X03_FORTYFIVE_FADE_OUT,
    TITLE_STATE_0X04_ADX_FADE_IN,
    TITLE_STATE_0X05_ADX,
    TITLE_STATE_0X06_ADX_FADE_OUT,
    TITLE_STATE_0X07_VMU_WARNING_FADE_IN,
    TITLE_STATE_0X08_VMU_WARNING,
    TITLE_STATE_0X09_VMU_WARNING_FADE_OUT,
    TITLE_STATE_0X0A_TITLE_FADE_IN,
    TITLE_STATE_0X0B_BUS_SLIDE,
    TITLE_STATE_0X0C_FLAG_REVEAL,
    TITLE_STATE_0X0D_TITLE_FADE_IN_DIRECT,
    TITLE_STATE_0X0E_PRESS_START,
    TITLE_STATE_0X0F_START_PRESSED,
    TITLE_STATE_0X10_START_PRESSED_FADE_OUT,
    TITLE_STATE_0X11_TIME_OUT
};
typedef enum TITLE_STATE TITLE_STATE;

/* TODO: move ResourceGroup/ResourceGroupInfo to a header of their own. */
struct ResourceGroupInfo {
    char* parts;
    char* dat;
    char* pvm;
    Uint32 tex_count;
}
typedef ResourceGroupInfo;

struct ResourceGroup {
    NJS_TEXLIST *tlist_0x00;
    NJS_TEXANIM *tanim_0x04;
    void *contents_0x08;
}
typedef ResourceGroup;

struct MenuState {
    ResourceGroup resourceGroupA_0x00;
    ResourceGroup resourceGroupB_0x0c;
    /* Typed after the title's enum, but every screen puts its own state
     * machine here. */
    TITLE_STATE state_0x18;
    int subState_0x1c;
    union {
        struct {
            float busX_0x20;
            float flagY_0x24;
        } title;
        struct {
            NJS_POINT2 cursor_0x20;
            NJS_POINT2 cursorTarget_0x28;
        } vmSelect;
        struct {
            NJS_POINT2 cursor_0x20;
            NJS_POINT2 cursorTarget_0x28;
        } cursor;
    } pos;
    NJS_POINT2 cursorVelocity_0x30;
    int selected_0x38;
    /* Shared menu cursor: 0x3c steps on left/right, 0x40 on up/down; each
     * screen picks what the axes index. A screen on its way out parks the
     * pair on the course-menu button it wants selected next. */
    int cursorCol_0x3c;
    int cursorRow_0x40;
    int scrollTopRow_0x44;
    int cursorVisible_0x48;
    int field_0x4c;  /* nothing in the tree reads or writes it */
    int courseId_0x50;
    /* Per-screen scratch with no shared meaning. */
    int field_0x54;
    int field_0x58;
    int field_0x5c;
    int instructorSprite_0x60;
    /* 0x68 is the frame tick everywhere; 0x64 is a per-screen step or page
     * index, and the title's PRESS START timeout. */
    int counter_0x64;
    int timer_0x68;
    int selectedVmuSlot_0x6c;  /* except in 01d7fc, which caches a VMU status here */
    int returnAction_0x70;
    int returnActionArg_0x74;
    BACKUPINFO* bupInfo_0x78;
}
typedef MenuState;

extern MenuState var_menuState_8c1bc7a8;
extern ResourceGroupInfo init_titleResourceGroup_8c044254;
extern ResourceGroupInfo init_mainMenuResourceGroup_8c044264;
extern ResourceGroupInfo init_practice01ResourceGroup_8c044274;
extern ResourceGroupInfo init_practice02ResourceGroup_8c044284;

/* direct: skip the logo sequence and fade straight into the title. */
void TitleStart_8c015fd6(Bool direct);

#endif /* _TITLE_H_ */
