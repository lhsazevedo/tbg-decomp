#include <shinobi.h>
#include "011120_asset_queues.h"
#include "014a9c_tasks.h"
#include "012504_input.h"
#include "serial_debug.h"

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
extern int var_activeCtrlType_8c157a70;
extern int var_resetRequested_8c157a78;
extern Uint32 var_vibport_8c1ba354;
extern PDS_PERIPHERAL *var_peripheral_8c1ba358;
extern PDS_PERIPHERAL var_peripherals_8c1ba35c[2];
/* var_8c1bbc4c: paddle remap timer (must be 0); var_8c1bbcc4: remap target (5=Up, 0=Down) */
extern float var_8c1bbc4c;
extern int var_8c1bbcc4;
extern Task var_tasks_8c1ba3c8[];
extern int var_8c157ae4;
extern int var_8c157ae8;
extern int var_8c157ad4[4];
extern int var_inputMapSel_8c1bb8c8;
extern void PspTask_8c012324(void);

STATIC char var_name_8c157aec[12];
STATIC const char *init_fortyFive_8c03bf40 = "FortyFive";

void inputTask_8c012504(void)
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
        LOG_TRACE(("[INPUT] inputTask_8c012504: no supported controller on port 0\n"));
        *var_peripherals_8c1ba35c = const_peripheral_8c033318;
        var_vibport_8c1ba354 = -1;
        var_activeCtrlType_8c157a70 = -1;
        vmsLcd_8c01c910();
        return;
    }

    var_peripherals_8c1ba35c[0].r = var_peripheral_8c1ba358->r;
    var_peripherals_8c1ba35c[0].l = var_peripheral_8c1ba358->l;
    var_peripherals_8c1ba35c[0].x1 = var_peripheral_8c1ba358->x1;
    var_peripherals_8c1ba35c[0].on = 0;
    var_peripherals_8c1ba35c[0].press = 0;
    var_activeCtrlType_8c157a70 = support;

    if (support == BT_CONTROLLER) {
        LOG_TRACE(("[INPUT] inputTask_8c012504: standard controller\n"));
        for (i = 0; i < 7; i++) {
            if (var_peripheral_8c1ba358->on & init_btnRemap_8c03be80[i].physical_0x00) {
                var_peripherals_8c1ba35c[0].on |= init_btnRemap_8c03be80[i].logical_0x04;
            }
        }
        for (i = 0; i < 7; i++) {
            if (var_peripheral_8c1ba358->press & init_btnRemap_8c03be80[i].physical_0x00) {
                var_peripherals_8c1ba35c[0].press |= init_btnRemap_8c03be80[i].logical_0x04;
            }
        }
        /* Sega mandatory reset combo: Start + A + B + X + Y */
        if ((var_peripheral_8c1ba358->press & PDD_DGT_ST) &&
            (var_peripheral_8c1ba358->on & (PDD_DGT_TA | PDD_DGT_TB | PDD_DGT_TX | PDD_DGT_TY)) ==
                (PDD_DGT_TA | PDD_DGT_TB | PDD_DGT_TX | PDD_DGT_TY)) {
            LOG_DEBUG(("[INPUT] inputTask_8c012504: soft-reset combo held\n"));
            var_resetRequested_8c157a78 = 1;
        }
    } else if (support == BT_RACING) {
        LOG_TRACE(("[INPUT] inputTask_8c012504: racing wheel\n"));
        for (i = 0; i < 5; i++) {
            if (var_peripheral_8c1ba358->on & init_btnRemapWheel_8c03bef0[i].physical_0x00) {
                var_peripherals_8c1ba35c[0].on |= init_btnRemapWheel_8c03bef0[i].logical_0x04;
            }
        }
        for (i = 0; i < 5; i++) {
            if (var_peripheral_8c1ba358->press & init_btnRemapWheel_8c03bef0[i].physical_0x00) {
                var_peripherals_8c1ba35c[0].press |= init_btnRemapWheel_8c03bef0[i].logical_0x04;
            }
        }
        /* Y + half-brake while timer idle -> remap Y to D-pad; else check reset combo */
        if ((var_peripherals_8c1ba35c[0].press & PDD_DGT_TY) &&
            var_8c1bbc4c == 0.0f &&
            var_peripherals_8c1ba35c[0].l >= 0x81) {
            LOG_DEBUG(("[INPUT] inputTask_8c012504: paddle-shift remap (mode %d)\n", var_8c1bbcc4));
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
                LOG_DEBUG(("[INPUT] inputTask_8c012504: soft-reset combo held (wheel)\n"));
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

    vmsLcd_8c01c910();
}

/* Like inputTask_8c012504 but uses alt translation tables and skips paddle-shift remap. */
void inputTaskAlt_8c012718(void)
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
        LOG_TRACE(("[INPUT] inputTaskAlt_8c012718: no supported controller on port 0\n"));
        *var_peripherals_8c1ba35c = const_peripheral_8c033318;
        var_vibport_8c1ba354 = -1;
        var_activeCtrlType_8c157a70 = -1;
        vmsLcd_8c01c910();
        return;
    }

    var_peripherals_8c1ba35c[0].r = var_peripheral_8c1ba358->r;
    var_peripherals_8c1ba35c[0].l = var_peripheral_8c1ba358->l;
    var_peripherals_8c1ba35c[0].x1 = var_peripheral_8c1ba358->x1;
    var_peripherals_8c1ba35c[0].on = 0;
    var_peripherals_8c1ba35c[0].press = 0;
    var_activeCtrlType_8c157a70 = support;

    if (support == BT_CONTROLLER) {
        LOG_TRACE(("[INPUT] inputTaskAlt_8c012718: standard controller\n"));
        for (i = 0; i < 7; i++) {
            if (var_peripheral_8c1ba358->on & init_btnRemapAlt_8c03beb8[i].physical_0x00) {
                var_peripherals_8c1ba35c[0].on |= init_btnRemapAlt_8c03beb8[i].logical_0x04;
            }
        }
        for (i = 0; i < 7; i++) {
            if (var_peripheral_8c1ba358->press & init_btnRemapAlt_8c03beb8[i].physical_0x00) {
                var_peripherals_8c1ba35c[0].press |= init_btnRemapAlt_8c03beb8[i].logical_0x04;
            }
        }
        /* Sega mandatory reset combo: Start + A + B + X + Y */
        if ((var_peripheral_8c1ba358->press & PDD_DGT_ST) &&
            (var_peripheral_8c1ba358->on & (PDD_DGT_TA | PDD_DGT_TB | PDD_DGT_TX | PDD_DGT_TY)) ==
                (PDD_DGT_TA | PDD_DGT_TB | PDD_DGT_TX | PDD_DGT_TY)) {
            LOG_DEBUG(("[INPUT] inputTaskAlt_8c012718: soft-reset combo held\n"));
            var_resetRequested_8c157a78 = 1;
        }
    } else if (support == BT_RACING) {
        LOG_TRACE(("[INPUT] inputTaskAlt_8c012718: racing wheel\n"));
        for (i = 0; i < 5; i++) {
            if (var_peripheral_8c1ba358->on & init_btnRemapWheelAlt_8c03bf18[i].physical_0x00) {
                var_peripherals_8c1ba35c[0].on |= init_btnRemapWheelAlt_8c03bf18[i].logical_0x04;
            }
        }
        for (i = 0; i < 5; i++) {
            if (var_peripheral_8c1ba358->press & init_btnRemapWheelAlt_8c03bf18[i].physical_0x00) {
                var_peripherals_8c1ba35c[0].press |= init_btnRemapWheelAlt_8c03bf18[i].logical_0x04;
            }
        }
        /* Sega reset combo for racing wheel: Start + A + B */
        if ((var_peripheral_8c1ba358->press & PDD_DGT_ST) &&
            (var_peripheral_8c1ba358->on & (PDD_DGT_TA | PDD_DGT_TB)) ==
                (PDD_DGT_TA | PDD_DGT_TB)) {
            LOG_DEBUG(("[INPUT] inputTaskAlt_8c012718: soft-reset combo held (wheel)\n"));
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

    vmsLcd_8c01c910();
}

/* param 0: install peripheral-support task, clear auto-fire state;
 * param 1: install the mapped input handler. */
void pushInputTask_8c0128cc(int param)
{
    void (*action)(void);
    void *created_state;

    if (param == 0) {
        LOG_DEBUG(("[INPUT] pushInputTask_8c0128cc: queueing peripheral-support task\n"));
        pushTask_8c014ae8(var_tasks_8c1ba3c8, PspTask_8c012324,
                          &var_8c157a74, &created_state, 0);
        var_8c157ae4 = 0;
        var_8c157ae8 = 0;
        var_8c157ad4[0] = 0;
    } else if (param == 1) {
        if (var_inputMapSel_8c1bb8c8 == 0) {
            action = inputTask_8c012504;
        } else {
            action = inputTaskAlt_8c012718;
        }
        LOG_DEBUG(("[INPUT] pushInputTask_8c0128cc: queueing input handler (%s)\n",
                   var_inputMapSel_8c1bb8c8 == 0 ? "inputTask_8c012504" : "inputTaskAlt_8c012718"));
        pushTask_8c014ae8(var_tasks_8c1ba3c8, action,
                          &var_8c157a74, &created_state, 0);
    }
}

/* Tail call in asm; dispatches inputTask or Alt via var_inputMapSel_8c1bb8c8. */
void dispatchInputTask_8c012970(void)
{
    if (var_inputMapSel_8c1bb8c8 == 0) {
        LOG_TRACE(("[INPUT] dispatchInputTask_8c012970: dispatch inputTask_8c012504\n"));
        inputTask_8c012504();
    } else {
        LOG_TRACE(("[INPUT] dispatchInputTask_8c012970: dispatch inputTaskAlt_8c012718\n"));
        inputTaskAlt_8c012718();
    }
}

/* Returns 1 if name was just initialized, 0 if already "FortyFive". */
int setName_8c012984(void)
{
    if (strcmp(var_name_8c157aec, init_fortyFive_8c03bf40) == 0) {
        LOG_DEBUG(("[INPUT] setName_8c012984: name already \"%s\"\n", var_name_8c157aec));
        return 0;
    }
    LOG_DEBUG(("[INPUT] setName_8c012984: setting name to \"%s\"\n", init_fortyFive_8c03bf40));
    strcpy(var_name_8c157aec, init_fortyFive_8c03bf40);
    return 1;
}
