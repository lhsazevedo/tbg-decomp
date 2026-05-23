#include <shinobi.h>
#include "014a9c_tasks.h"

#define BT(a)     PDD_DEV_SUPPORT_##a

/* Dreamcast controller */
#define BT_CONTROLLER   (BT(TA)  | BT(TB)  | BT(TX)  | BT(TY)  | BT(ST)  | \
                         BT(KU)  | BT(KD)  | BT(KL)  | BT(KR)  | \
                         BT(AX1) | BT(AY1) | BT(AL)  | BT(AR))

/* Racing controller */
#define BT_RACING       (BT(TA)  | BT(TB)  | BT(ST)  | \
                         BT(KU)  | BT(KD)  | BT(KL)  | BT(KR)  | \
                         BT(AX1) | BT(AL)  | BT(AR))

extern PDS_PERIPHERAL const_peripheral_8c033318;
extern int var_8c157a70;
extern int var_resetRequested_8c157a78;
extern Uint32 var_vibport_8c1ba354;
extern PDS_PERIPHERAL *var_peripheral_8c1ba358;
extern PDS_PERIPHERAL var_peripherals_8c1ba35c[2];
extern int init_8c03be80[14];
extern int init_8c03bef0[10];
extern int init_8c03beb8[14];
extern int init_8c03bf18[10];
/* Racing-wheel state. Names unknown; var_8c1bbc4c gates the paddle-shift
 * remap (likely a timer that must be 0); var_8c1bbcc4 picks the remap
 * target (5 -> D-pad Up, 0 -> D-pad Down). */
extern float var_8c1bbc4c;
extern int var_8c1bbcc4;
extern Task var_tasks_8c1ba3c8[];
extern Task *var_8c157a74;
extern int var_8c157ae4;
extern int var_8c157ae8;
extern int var_8c157ad4[4];
extern int var_8c1bb8c8;
extern void PspTask_8c012324(void);

void task_8c012504(void)
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
        *var_peripherals_8c1ba35c = const_peripheral_8c033318;
        var_vibport_8c1ba354 = -1;
        var_8c157a70 = -1;
        vmsLcd_8c01c910();
        return;
    }

    var_peripherals_8c1ba35c[0].r = var_peripheral_8c1ba358->r;
    var_peripherals_8c1ba35c[0].l = var_peripheral_8c1ba358->l;
    var_peripherals_8c1ba35c[0].x1 = var_peripheral_8c1ba358->x1;
    var_peripherals_8c1ba35c[0].on = 0;
    var_peripherals_8c1ba35c[0].press = 0;
    var_8c157a70 = support;

    if (support == BT_CONTROLLER) {
        for (i = 0; i < 14; i += 2) {
            if (var_peripheral_8c1ba358->on & init_8c03be80[i]) {
                var_peripherals_8c1ba35c[0].on |= init_8c03be80[i + 1];
            }
        }
        for (i = 0; i < 14; i += 2) {
            if (var_peripheral_8c1ba358->press & init_8c03be80[i]) {
                var_peripherals_8c1ba35c[0].press |= init_8c03be80[i + 1];
            }
        }
        /* Sega mandatory reset combo: Start + A + B + X + Y */
        if ((var_peripheral_8c1ba358->press & PDD_DGT_ST) &&
            (var_peripheral_8c1ba358->on & (PDD_DGT_TA | PDD_DGT_TB | PDD_DGT_TX | PDD_DGT_TY)) ==
                (PDD_DGT_TA | PDD_DGT_TB | PDD_DGT_TX | PDD_DGT_TY)) {
            var_resetRequested_8c157a78 = 1;
        }
    } else if (support == BT_RACING) {
        for (i = 0; i < 10; i += 2) {
            if (var_peripheral_8c1ba358->on & init_8c03bef0[i]) {
                var_peripherals_8c1ba35c[0].on |= init_8c03bef0[i + 1];
            }
        }
        for (i = 0; i < 10; i += 2) {
            if (var_peripheral_8c1ba358->press & init_8c03bef0[i]) {
                var_peripherals_8c1ba35c[0].press |= init_8c03bef0[i + 1];
            }
        }
        /* Paddle-shift remap: when the Y-paddle is freshly pressed while the
         * wheel is idle (timer == 0) and the left brake is at least half-
         * applied, remap Y to a D-pad press (Up in mode 5, Down in mode 0).
         * Otherwise fall through to the racing-controller reset combo. */
        if ((var_peripherals_8c1ba35c[0].press & PDD_DGT_TY) &&
            var_8c1bbc4c == 0.0f &&
            var_peripherals_8c1ba35c[0].l >= 0x81) {
            if (var_8c1bbcc4 == 5) {
                var_peripherals_8c1ba35c[0].press ^= (PDD_DGT_TY | PDD_DGT_KU);
            } else if (var_8c1bbcc4 == 0) {
                var_peripherals_8c1ba35c[0].press ^= (PDD_DGT_TY | PDD_DGT_KD);
            }
        } else {
            /* Sega reset combo for racing wheel: Start + A + B */
            if ((var_peripheral_8c1ba358->press & PDD_DGT_ST) &&
                (var_peripheral_8c1ba358->on & (PDD_DGT_TA | PDD_DGT_TB)) ==
                    (PDD_DGT_TA | PDD_DGT_TB)) {
                var_resetRequested_8c157a78 = 1;
            }
        }
    }

    /* Pick the port that has a vibration pack: prefer port 1, then port 2;
     * -1 if neither has one. */
    if (pdGetPeripheral(1)->info->type & PDD_DEVTYPE_VIBRATION) {
        var_vibport_8c1ba354 = 1;
    } else if (pdGetPeripheral(2)->info->type & PDD_DEVTYPE_VIBRATION) {
        var_vibport_8c1ba354 = 2;
    } else {
        var_vibport_8c1ba354 = -1;
    }

    vmsLcd_8c01c910();
}

/* Alternate input handler, selected over task_8c012504 by FUN_8c012970 based
 * on var_8c1bb8c8. Identical flow but reads the adjacent translation tables
 * (init_8c03beb8 / init_8c03bf18) and has no paddle-shift remap. */
void FUN_8c012718(void)
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
        *var_peripherals_8c1ba35c = const_peripheral_8c033318;
        var_vibport_8c1ba354 = -1;
        var_8c157a70 = -1;
        vmsLcd_8c01c910();
        return;
    }

    var_peripherals_8c1ba35c[0].r = var_peripheral_8c1ba358->r;
    var_peripherals_8c1ba35c[0].l = var_peripheral_8c1ba358->l;
    var_peripherals_8c1ba35c[0].x1 = var_peripheral_8c1ba358->x1;
    var_peripherals_8c1ba35c[0].on = 0;
    var_peripherals_8c1ba35c[0].press = 0;
    var_8c157a70 = support;

    if (support == BT_CONTROLLER) {
        for (i = 0; i < 14; i += 2) {
            if (var_peripheral_8c1ba358->on & init_8c03beb8[i]) {
                var_peripherals_8c1ba35c[0].on |= init_8c03beb8[i + 1];
            }
        }
        for (i = 0; i < 14; i += 2) {
            if (var_peripheral_8c1ba358->press & init_8c03beb8[i]) {
                var_peripherals_8c1ba35c[0].press |= init_8c03beb8[i + 1];
            }
        }
        /* Sega mandatory reset combo: Start + A + B + X + Y */
        if ((var_peripheral_8c1ba358->press & PDD_DGT_ST) &&
            (var_peripheral_8c1ba358->on & (PDD_DGT_TA | PDD_DGT_TB | PDD_DGT_TX | PDD_DGT_TY)) ==
                (PDD_DGT_TA | PDD_DGT_TB | PDD_DGT_TX | PDD_DGT_TY)) {
            var_resetRequested_8c157a78 = 1;
        }
    } else if (support == BT_RACING) {
        for (i = 0; i < 10; i += 2) {
            if (var_peripheral_8c1ba358->on & init_8c03bf18[i]) {
                var_peripherals_8c1ba35c[0].on |= init_8c03bf18[i + 1];
            }
        }
        for (i = 0; i < 10; i += 2) {
            if (var_peripheral_8c1ba358->press & init_8c03bf18[i]) {
                var_peripherals_8c1ba35c[0].press |= init_8c03bf18[i + 1];
            }
        }
        /* Sega reset combo for racing wheel: Start + A + B */
        if ((var_peripheral_8c1ba358->press & PDD_DGT_ST) &&
            (var_peripheral_8c1ba358->on & (PDD_DGT_TA | PDD_DGT_TB)) ==
                (PDD_DGT_TA | PDD_DGT_TB)) {
            var_resetRequested_8c157a78 = 1;
        }
    }

    /* Pick the port that has a vibration pack: prefer port 1, then port 2;
     * -1 if neither has one. */
    if (pdGetPeripheral(1)->info->type & PDD_DEVTYPE_VIBRATION) {
        var_vibport_8c1ba354 = 1;
    } else if (pdGetPeripheral(2)->info->type & PDD_DEVTYPE_VIBRATION) {
        var_vibport_8c1ba354 = 2;
    } else {
        var_vibport_8c1ba354 = -1;
    }

    vmsLcd_8c01c910();
}

/* Queues the active input-handler task. param 0 installs the peripheral-support
 * task (PspTask_8c012324) and clears its repeat/auto-fire state; param 1 picks
 * task_8c012504 or FUN_8c012718 by var_8c1bb8c8. */
void FUN_8c0128cc(int param)
{
    void (*action)(void);
    void *created_state;

    if (param == 0) {
        pushTask_8c014ae8(var_tasks_8c1ba3c8, PspTask_8c012324,
                          &var_8c157a74, &created_state, 0);
        var_8c157ae4 = 0;
        var_8c157ae8 = 0;
        var_8c157ad4[0] = 0;
    } else if (param == 1) {
        if (var_8c1bb8c8 == 0) {
            action = task_8c012504;
        } else {
            action = FUN_8c012718;
        }
        pushTask_8c014ae8(var_tasks_8c1ba3c8, action,
                          &var_8c157a74, &created_state, 0);
    }
}

/* Task action that runs the active input handler each frame: task_8c012504
 * or FUN_8c012718, selected by var_8c1bb8c8. (In asm this is a tail call.) */
void FUN_8c012970(void)
{
    if (var_8c1bb8c8 == 0) {
        task_8c012504();
    } else {
        FUN_8c012718();
    }
}
