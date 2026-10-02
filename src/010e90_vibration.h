/* 8c010e90 - Vibration */
#ifndef _010E90_VIBRATION_H
#define _010E90_VIBRATION_H

#include <shinobi.h>

/* Copied over the pad state when the controller is gone. */
extern const PDS_PERIPHERAL const_peripheralZero_8c033318;

void VibClear_8c010fbe();
/* Pattern 0-7; only a higher index displaces one already playing. */
void VibStart_8c010f7a(int pattern);
void VibUpdate_8c010fae(int port);

#endif // _010E90_VIBRATION_H
