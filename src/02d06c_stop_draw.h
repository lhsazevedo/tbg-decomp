#ifndef _02D06C_STOP_DRAW_H
#define _02D06C_STOP_DRAW_H

void StopDrawWaitingPassengers_8c02d06c(
    /* 0 for the main view, 1 for the mirror; picks the passenger's facing */
    int layer
);

/* Bracket the in-bus passenger draws with the light and fade material. */
void StopDrawLightBegin_8c02d0fc(int arg0);
void StopDrawLightEnd_8c02d146(int arg0);

#endif // _02D06C_STOP_DRAW_H
