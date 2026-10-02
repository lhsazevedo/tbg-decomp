/* @unit Input */
#include <shinobi.h>
#include "010e90_vibration.h"
#include "011120_asset_queues.h"
#include "012324_input.h"
#include "014a9c_tasks.h"
#include "01bb48_vm_game.h"
#include "1ba1c8_globals.h"
#include "includes.h" /* STATIC */
#include "serial_debug.h"

/* ====================
 * Compiler Definitions
 * ====================
 */

#define DGT_ABXY (PDD_DGT_TA | PDD_DGT_TB | PDD_DGT_TX | PDD_DGT_TY)
#define DGT_UDLR (PDD_DGT_KU | PDD_DGT_KD | PDD_DGT_KL | PDD_DGT_KR)

/* Stick deflection that counts as a dpad press. */
#define STICK_THRESHOLD 64

#define REPEAT_DELAY    15 /* frames held before the first repeat */
#define REPEAT_PERIOD    6 /* and between repeats after that */
#define REPEAT_RAMP     30 /* held this long, it goes to every frame */

/* =================
 * Type Declarations
 * =================
 */

/* Menu dpad auto-repeat. */
typedef struct {
    int active_0x00;
    int counter_0x04;
    int period_0x08;
    int totalFrames_0x0c;
} KeyRepeat;

/* =======================
 * Non-initialized Globals
 * =======================
 */

STATIC KeyRepeat var_keyRepeat_8c157ad4;

/* Last dpad direction the analog stick stood for, so a held stick presses once. */
STATIC int var_stickLatchX_8c157ae4;
STATIC int var_stickLatchY_8c157ae8;

STATIC char var_bootSentinel_8c157aec[12];

/* ===================
 * Initialized Globals
 * ===================
 */

STATIC const char *init_fortyFive_8c03bf40 = "FortyFive";

/* =========
 * Functions
 * =========
 */

/* Port 0 as is, plus dpad presses from the analog stick and dpad key repeat. */
STATIC void inputMenuTask_8c012324()
{
    int support;

    var_resetRequested_8c157a78 = 0;
    var_peripheral_8c1ba358 = pdGetPeripheral(0);

    support = var_peripheral_8c1ba358->support & BT_CONTROLLER;
    if (
        (var_peripheral_8c1ba358->info->type & PDD_DEVTYPE_CONTROLLER) &&
        (support == BT_CONTROLLER || support == BT_RACING)
    ) {
        var_peripherals_8c1ba35c[0] = *var_peripheral_8c1ba358;
        if (support == BT_CONTROLLER) {
            var_activeCtrlType_8c157a70 = BT_CONTROLLER;
            /* The racing controller has no stick to fold in. */
            if (var_peripherals_8c1ba35c[0].x1 < -STICK_THRESHOLD) {
                var_peripherals_8c1ba35c[0].on |= PDD_DGT_KL;
                if (!(var_stickLatchX_8c157ae4 & PDD_DGT_KL)) {
                    var_peripherals_8c1ba35c[0].press |= PDD_DGT_KL;
                }
                var_stickLatchX_8c157ae4 = PDD_DGT_KL;
            }
            else if (var_peripherals_8c1ba35c[0].x1 > STICK_THRESHOLD) {
                var_peripherals_8c1ba35c[0].on |= PDD_DGT_KR;
                if (!(var_stickLatchX_8c157ae4 & PDD_DGT_KR)) {
                    var_peripherals_8c1ba35c[0].press |= PDD_DGT_KR;
                }
                var_stickLatchX_8c157ae4 = PDD_DGT_KR;
            }
            else {
                var_stickLatchX_8c157ae4 = 0;
            }

            if (var_peripherals_8c1ba35c[0].y1 < -STICK_THRESHOLD) {
                var_peripherals_8c1ba35c[0].on |= PDD_DGT_KU;
                if (!(var_stickLatchY_8c157ae8 & PDD_DGT_KU)) {
                    var_peripherals_8c1ba35c[0].press |= PDD_DGT_KU;
                }
                var_stickLatchY_8c157ae8 = PDD_DGT_KU;
            }
            else if (var_peripherals_8c1ba35c[0].y1 > STICK_THRESHOLD) {
                var_peripherals_8c1ba35c[0].on |= PDD_DGT_KD;
                if (!(var_stickLatchY_8c157ae8 & PDD_DGT_KD)) {
                    var_peripherals_8c1ba35c[0].press |= PDD_DGT_KD;
                }
                var_stickLatchY_8c157ae8 = PDD_DGT_KD;
            }
            else {
                var_stickLatchY_8c157ae8 = 0;
            }

            /* Sega's mandatory soft-reset combo. */
            if (
                (var_peripheral_8c1ba358->press & PDD_DGT_ST) &&
                ((var_peripheral_8c1ba358->on & DGT_ABXY) == DGT_ABXY)
            ) {
                var_resetRequested_8c157a78 = 1;
            }
        } else if (support == BT_RACING) {
            var_activeCtrlType_8c157a70 = BT_RACING;
            /* The standard combo, unreachable on a wheel: it has no X/Y. */
            if (
                (var_peripheral_8c1ba358->press & PDD_DGT_ST) &&
                ((var_peripheral_8c1ba358->on & DGT_ABXY) == DGT_ABXY)
            ) {
                var_resetRequested_8c157a78 = 1;
            }
        }
    }
    else {
        *var_peripherals_8c1ba35c = const_peripheralZero_8c033318;
        var_vibport_8c1ba354 = -1;
        var_activeCtrlType_8c157a70 = -1;
    }

    if (var_keyRepeat_8c157ad4.active_0x00 == 0) {
        if (var_peripherals_8c1ba35c[0].on & DGT_UDLR) {
            var_keyRepeat_8c157ad4.active_0x00 = 1;
            var_keyRepeat_8c157ad4.counter_0x04 = 0;
            var_keyRepeat_8c157ad4.period_0x08 = REPEAT_DELAY;
            var_keyRepeat_8c157ad4.totalFrames_0x0c = 0;
        }
    } else if (var_keyRepeat_8c157ad4.active_0x00 == 1) {
        if (!(var_peripherals_8c1ba35c[0].on & DGT_UDLR)) {
            var_keyRepeat_8c157ad4.active_0x00 = 0;
        } else {
            if (var_keyRepeat_8c157ad4.period_0x08 <= ++var_keyRepeat_8c157ad4.counter_0x04) {
                var_keyRepeat_8c157ad4.counter_0x04 = 0;
                var_keyRepeat_8c157ad4.period_0x08 = REPEAT_PERIOD;
                var_peripherals_8c1ba35c[0].press |=
                    var_peripherals_8c1ba35c[0].on & DGT_UDLR;
            }
            if (++var_keyRepeat_8c157ad4.totalFrames_0x0c > REPEAT_RAMP) {
                var_keyRepeat_8c157ad4.period_0x08 = 1;
            }
        }
    }

    VmGameUpdateLcd_8c01c910();
    SndUpdateAdxVolFade_8c010a40();
}

/* Drive input for manual transmission, through the configurable remap tables. */
STATIC void inputManualTask_8c012504(void)
{
    int support;
    int i;

    var_resetRequested_8c157a78 = 0;
    var_peripheral_8c1ba358 = pdGetPeripheral(0);

    support = var_peripheral_8c1ba358->support & BT_CONTROLLER;
    if (
        !(var_peripheral_8c1ba358->info->type & PDD_DEVTYPE_CONTROLLER) ||
        (support != BT_CONTROLLER && support != BT_RACING)
    ) {
        LOG_TRACE(("[INPUT] inputManualTask_8c012504: no supported controller on port 0\n"));
        *var_peripherals_8c1ba35c = const_peripheralZero_8c033318;
        var_vibport_8c1ba354 = -1;
        var_activeCtrlType_8c157a70 = -1;
        VmGameUpdateLcd_8c01c910();
        return;
    }

    var_peripherals_8c1ba35c[0].r = var_peripheral_8c1ba358->r;
    var_peripherals_8c1ba35c[0].l = var_peripheral_8c1ba358->l;
    var_peripherals_8c1ba35c[0].x1 = var_peripheral_8c1ba358->x1;
    var_peripherals_8c1ba35c[0].on = 0;
    var_peripherals_8c1ba35c[0].press = 0;
    var_activeCtrlType_8c157a70 = support;

    if (support == BT_CONTROLLER) {
        LOG_TRACE(("[INPUT] inputManualTask_8c012504: standard controller\n"));
        for (i = 0; i < 7; i++) {
            if (var_peripheral_8c1ba358->on & init_btnRemapManual_8c03be80[i].physical_0x00) {
                var_peripherals_8c1ba35c[0].on |= init_btnRemapManual_8c03be80[i].logical_0x04;
            }
        }
        for (i = 0; i < 7; i++) {
            if (var_peripheral_8c1ba358->press & init_btnRemapManual_8c03be80[i].physical_0x00) {
                var_peripherals_8c1ba35c[0].press |= init_btnRemapManual_8c03be80[i].logical_0x04;
            }
        }
        /* Sega's mandatory soft-reset combo. */
        if ((var_peripheral_8c1ba358->press & PDD_DGT_ST) &&
            (var_peripheral_8c1ba358->on & (PDD_DGT_TA | PDD_DGT_TB | PDD_DGT_TX | PDD_DGT_TY)) ==
                (PDD_DGT_TA | PDD_DGT_TB | PDD_DGT_TX | PDD_DGT_TY)) {
            LOG_DEBUG(("[INPUT] inputManualTask_8c012504: soft-reset combo held\n"));
            var_resetRequested_8c157a78 = 1;
        }
    } else if (support == BT_RACING) {
        LOG_TRACE(("[INPUT] inputManualTask_8c012504: racing wheel\n"));
        for (i = 0; i < 5; i++) {
            if (var_peripheral_8c1ba358->on & init_btnRemapWheelManual_8c03bef0[i].physical_0x00) {
                var_peripherals_8c1ba35c[0].on |= init_btnRemapWheelManual_8c03bef0[i].logical_0x04;
            }
        }
        for (i = 0; i < 5; i++) {
            if (var_peripheral_8c1ba358->press & init_btnRemapWheelManual_8c03bef0[i].physical_0x00) {
                var_peripherals_8c1ba35c[0].press |= init_btnRemapWheelManual_8c03bef0[i].logical_0x04;
            }
        }
        /* Forward/reverse selector: at a standstill with the brake past
           half, Y becomes the dpad press updateGearSelector_8c0242ce reads --
           Up to leave reverse, Down to enter it. */
        if ((var_peripherals_8c1ba35c[0].press & PDD_DGT_TY) &&
            var_busState_8c1bb9d0.speed_0x27c == 0.0f &&
            var_peripherals_8c1ba35c[0].l >= 0x81) {
            LOG_DEBUG(("[INPUT] inputManualTask_8c012504: gear selector (gear %d)\n", var_busState_8c1bb9d0.gear_0x2f4));
            if (var_busState_8c1bb9d0.gear_0x2f4 == 5) {
                var_peripherals_8c1ba35c[0].press ^= (PDD_DGT_TY | PDD_DGT_KU);
            } else if (var_busState_8c1bb9d0.gear_0x2f4 == 0) {
                var_peripherals_8c1ba35c[0].press ^= (PDD_DGT_TY | PDD_DGT_KD);
            }
        } else {
            /* The wheel has no X/Y, so the combo is Start + A + B. */
            if ((var_peripheral_8c1ba358->press & PDD_DGT_ST) &&
                (var_peripheral_8c1ba358->on & (PDD_DGT_TA | PDD_DGT_TB)) ==
                    (PDD_DGT_TA | PDD_DGT_TB)) {
                LOG_DEBUG(("[INPUT] inputManualTask_8c012504: soft-reset combo held (wheel)\n"));
                var_resetRequested_8c157a78 = 1;
            }
        }
    }

    if (pdGetPeripheral(1)->info->type & PDD_DEVTYPE_VIBRATION) {
        var_vibport_8c1ba354 = 1;
    } else if (pdGetPeripheral(2)->info->type & PDD_DEVTYPE_VIBRATION) {
        var_vibport_8c1ba354 = 2;
    } else {
        var_vibport_8c1ba354 = -1;
    }

    VmGameUpdateLcd_8c01c910();
}

/* The same for automatic transmission, without the forward/reverse selector. */
STATIC void inputAutoTask_8c012718(void)
{
    int support;
    int i;

    var_resetRequested_8c157a78 = 0;
    var_peripheral_8c1ba358 = pdGetPeripheral(0);

    support = var_peripheral_8c1ba358->support & BT_CONTROLLER;
    if (
        !(var_peripheral_8c1ba358->info->type & PDD_DEVTYPE_CONTROLLER) ||
        (support != BT_CONTROLLER && support != BT_RACING)
    ) {
        LOG_TRACE(("[INPUT] inputAutoTask_8c012718: no supported controller on port 0\n"));
        *var_peripherals_8c1ba35c = const_peripheralZero_8c033318;
        var_vibport_8c1ba354 = -1;
        var_activeCtrlType_8c157a70 = -1;
        VmGameUpdateLcd_8c01c910();
        return;
    }

    var_peripherals_8c1ba35c[0].r = var_peripheral_8c1ba358->r;
    var_peripherals_8c1ba35c[0].l = var_peripheral_8c1ba358->l;
    var_peripherals_8c1ba35c[0].x1 = var_peripheral_8c1ba358->x1;
    var_peripherals_8c1ba35c[0].on = 0;
    var_peripherals_8c1ba35c[0].press = 0;
    var_activeCtrlType_8c157a70 = support;

    if (support == BT_CONTROLLER) {
        LOG_TRACE(("[INPUT] inputAutoTask_8c012718: standard controller\n"));
        for (i = 0; i < 7; i++) {
            if (var_peripheral_8c1ba358->on & init_btnRemapAuto_8c03beb8[i].physical_0x00) {
                var_peripherals_8c1ba35c[0].on |= init_btnRemapAuto_8c03beb8[i].logical_0x04;
            }
        }
        for (i = 0; i < 7; i++) {
            if (var_peripheral_8c1ba358->press & init_btnRemapAuto_8c03beb8[i].physical_0x00) {
                var_peripherals_8c1ba35c[0].press |= init_btnRemapAuto_8c03beb8[i].logical_0x04;
            }
        }
        /* Sega's mandatory soft-reset combo. */
        if ((var_peripheral_8c1ba358->press & PDD_DGT_ST) &&
            (var_peripheral_8c1ba358->on & (PDD_DGT_TA | PDD_DGT_TB | PDD_DGT_TX | PDD_DGT_TY)) ==
                (PDD_DGT_TA | PDD_DGT_TB | PDD_DGT_TX | PDD_DGT_TY)) {
            LOG_DEBUG(("[INPUT] inputAutoTask_8c012718: soft-reset combo held\n"));
            var_resetRequested_8c157a78 = 1;
        }
    } else if (support == BT_RACING) {
        LOG_TRACE(("[INPUT] inputAutoTask_8c012718: racing wheel\n"));
        for (i = 0; i < 5; i++) {
            if (var_peripheral_8c1ba358->on & init_btnRemapWheelAuto_8c03bf18[i].physical_0x00) {
                var_peripherals_8c1ba35c[0].on |= init_btnRemapWheelAuto_8c03bf18[i].logical_0x04;
            }
        }
        for (i = 0; i < 5; i++) {
            if (var_peripheral_8c1ba358->press & init_btnRemapWheelAuto_8c03bf18[i].physical_0x00) {
                var_peripherals_8c1ba35c[0].press |= init_btnRemapWheelAuto_8c03bf18[i].logical_0x04;
            }
        }
        /* The wheel has no X/Y, so the combo is Start + A + B. */
        if ((var_peripheral_8c1ba358->press & PDD_DGT_ST) &&
            (var_peripheral_8c1ba358->on & (PDD_DGT_TA | PDD_DGT_TB)) ==
                (PDD_DGT_TA | PDD_DGT_TB)) {
            LOG_DEBUG(("[INPUT] inputAutoTask_8c012718: soft-reset combo held (wheel)\n"));
            var_resetRequested_8c157a78 = 1;
        }
    }

    if (pdGetPeripheral(1)->info->type & PDD_DEVTYPE_VIBRATION) {
        var_vibport_8c1ba354 = 1;
    } else if (pdGetPeripheral(2)->info->type & PDD_DEVTYPE_VIBRATION) {
        var_vibport_8c1ba354 = 2;
    } else {
        var_vibport_8c1ba354 = -1;
    }

    VmGameUpdateLcd_8c01c910();
}

/* mode: 0 menus, 1 drive; anything else installs nothing. */
void InputInstall_8c0128cc(int mode)
{
    void (*action)(void);
    void *installed_state;

    if (mode == 0) {
        LOG_DEBUG(("[INPUT] InputInstall_8c0128cc: queueing inputMenuTask_8c012324\n"));
        TaskInstall_8c014ae8(var_tasks_8c1ba3c8, inputMenuTask_8c012324,
                          &var_pushedTask_8c157a74, &installed_state, 0);
        var_stickLatchX_8c157ae4 = 0;
        var_stickLatchY_8c157ae8 = 0;
        var_keyRepeat_8c157ad4.active_0x00 = 0;
    } else if (mode == 1) {
        if (var_driveMode_8c1bb8c8 == 0) {
            action = inputManualTask_8c012504;
        } else {
            action = inputAutoTask_8c012718;
        }
        LOG_DEBUG(("[INPUT] InputInstall_8c0128cc: queueing input handler (%s)\n",
                   var_driveMode_8c1bb8c8 == 0 ? "inputManualTask_8c012504" : "inputAutoTask_8c012718"));
        TaskInstall_8c014ae8(var_tasks_8c1ba3c8, action,
                          &var_pushedTask_8c157a74, &installed_state, 0);
    }
}

/* One frame of drive input, so the drive's first frame already sees mapped
 * buttons. */
void InputPollDrive_8c012970(void)
{
    if (var_driveMode_8c1bb8c8 == 0) {
        LOG_TRACE(("[INPUT] InputPollDrive_8c012970: dispatch inputManualTask_8c012504\n"));
        inputManualTask_8c012504();
    } else {
        LOG_TRACE(("[INPUT] InputPollDrive_8c012970: dispatch inputAutoTask_8c012718\n"));
        inputAutoTask_8c012718();
    }
}

/* True only on the first call since power-on: the sentinel lives in BSS, which
 * a return to the title leaves standing. */
int InputCheckColdBoot_8c012984(void)
{
    if (strcmp(var_bootSentinel_8c157aec, init_fortyFive_8c03bf40) == 0) {
        LOG_DEBUG(("[INPUT] InputCheckColdBoot_8c012984: name already \"%s\"\n", var_bootSentinel_8c157aec));
        return 0;
    }
    LOG_DEBUG(("[INPUT] InputCheckColdBoot_8c012984: setting name to \"%s\"\n", init_fortyFive_8c03bf40));
    strcpy(var_bootSentinel_8c157aec, init_fortyFive_8c03bf40);
    return 1;
}
