#ifndef _TITLE_H_
#define _TITLE_H_

#include <shinobi.h>
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

struct ResourceGroupInfo {
    char* parts;
    char* dat;
    char* pvm;
    Uint32 tex_count;
}
typedef ResourceGroupInfo;

// TODO: Move
struct ResourceGroup {
    NJS_TEXLIST *tlist_0x00;
    NJS_TEXANIM *tanim_0x04;
    void *contents_0x08;
}
typedef ResourceGroup;

struct MenuState {
    ResourceGroup resourceGroupA_0x00;
    ResourceGroup resourceGroupB_0x0c;
    TITLE_STATE state_0x18;
    int field_0x1c;
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
    int field_0x3c;
    int field_0x40;
    int field_0x44;
    int field_0x48;
    int field_0x4c;
    int field_0x50;
    int field_0x54;
    int field_0x58;
    int field_0x5c;
    int field_0x60;
    int startTimer_0x64;
    int logo_timer_0x68;
    int selectedVmuSlot_0x6c;
    int field_0x70;
    int field_0x74;
    BACKUPINFO* bupInfo_0x78;
}
typedef MenuState;

extern MenuState menuState_8c1bc7a8;
extern void* _8c225fb0;

#endif /* _TITLE_H_ */
