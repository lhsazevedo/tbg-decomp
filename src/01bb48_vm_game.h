#ifndef _VM_GAME_H
#define _VM_GAME_H

#include <shinobi.h>
#include "015ab8_title.h"

/* =================
 * Type Declarations
 * =================
 */

typedef struct {
    Uint8 data[0x600];
} LcdFrame;

typedef struct {
    Uint32 count;
    LcdFrame data[1]; // Placeholder for variable-length data
} LcdFrames;

typedef struct {
    LcdFrames *frames;
    int delay;
    int frameIdx;
} LcdAnim;

enum VmGameBupPhase {
    VMGAME_BUP_ERROR        = -1,
    VMGAME_BUP_IDLE         = 0,
    VMGAME_BUP_SAVE_BUSY    = 18,
    VMGAME_BUP_SAVE_DONE    = 19,
    VMGAME_BUP_DEFRAG_BUSY  = 20,
    VMGAME_BUP_DEFRAG_DONE  = 21,
    VMGAME_BUP_LOAD_BUSY    = 22,
    VMGAME_BUP_LOAD_DONE    = 23,
    VMGAME_BUP_REWRITE_BUSY = 24,
    VMGAME_BUP_REWRITE_DONE = 25
};

/* =======================
 * Non-initialized Globals
 * =======================
 */

extern int var_lcdAnimActive_8c2260a8;
extern LcdAnim var_lcdAnimBus_8c2260ac; // vm_bus.lcd anim
extern LcdAnim var_lcdAnimDanger_8c2260b8; // vm_danger.lcd anim
extern LcdAnim var_lcdAnimLoading_8c2260c4; // now_loading.lcd anim

/* =========
 * Functions
 * =========
 */

void VmGameEnter_8c01c880(Task *task);
void VmGameResetLcdAnims_8c01c8dc(void);
void VmGameSetLcdAnim_8c01c8fc(int idx);
void VmGameUpdateLcd_8c01c910(void);

#endif // _VM_GAME_H
