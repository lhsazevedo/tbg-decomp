#ifndef _020214_DRIVE_CUE_H
#define _020214_DRIVE_CUE_H

typedef struct {
    int ambientChimeState_0x00;
    int ambientChimeTimer_0x04;
    int stopAnnounceState_0x08;
    int announceRequested_0x0c;
    int stopAnnounceTimer_0x10;
    int segmentChimeLatch_0x14;
    int trafficChimeArmed_0x18;
} DriveCueState;

extern DriveCueState var_driveCueState_8c2264b8;

void DriveCueInit_8c020528();

#endif // _020214_DRIVE_CUE_H
