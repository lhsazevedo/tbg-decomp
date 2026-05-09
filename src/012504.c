#include <shinobi.h>

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

void task_8c012504(void)
{
    int support;

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
    } else {
        var_peripherals_8c1ba35c[0].r = var_peripheral_8c1ba358->r;
        var_peripherals_8c1ba35c[0].l = var_peripheral_8c1ba358->l;
        var_peripherals_8c1ba35c[0].x1 = var_peripheral_8c1ba358->x1;
        var_peripherals_8c1ba35c[0].on = 0;
        var_peripherals_8c1ba35c[0].press = 0;
        var_8c157a70 = support;
    }

    vmsLcd_8c01c910();
}
