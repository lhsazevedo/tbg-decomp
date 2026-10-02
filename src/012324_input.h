/* 8c012324 - Input */
#ifndef _INPUT_H
#define _INPUT_H

/*
 * The two values of PDS_PERIPHERAL.support the game accepts, masked with
 * BT_CONTROLLER and mirrored into var_activeCtrlType_8c157a70: 0xf06fe is the
 * standard pad, 0x700fe the racing controller (no X/Y, no second stick axis).
 */
#define BT_CONTROLLER   (PDD_DEV_SUPPORT_TA | PDD_DEV_SUPPORT_TB | PDD_DEV_SUPPORT_TX | \
                         PDD_DEV_SUPPORT_TY | PDD_DEV_SUPPORT_ST | PDD_DEV_SUPPORT_KU | \
                         PDD_DEV_SUPPORT_KD | PDD_DEV_SUPPORT_KL | PDD_DEV_SUPPORT_KR | \
                         PDD_DEV_SUPPORT_AX1 | PDD_DEV_SUPPORT_AY1 | PDD_DEV_SUPPORT_AL | \
                         PDD_DEV_SUPPORT_AR)

#define BT_RACING       (PDD_DEV_SUPPORT_TA | PDD_DEV_SUPPORT_TB | PDD_DEV_SUPPORT_ST | \
                         PDD_DEV_SUPPORT_KU | PDD_DEV_SUPPORT_KD | PDD_DEV_SUPPORT_KL | \
                         PDD_DEV_SUPPORT_KR | PDD_DEV_SUPPORT_AX1 | PDD_DEV_SUPPORT_AL | \
                         PDD_DEV_SUPPORT_AR)

void InputSpawnTask_8c0128cc(int mode);
void InputPollDrive_8c012970(void);
int InputCheckColdBoot_8c012984(void);

#endif // _INPUT_H
