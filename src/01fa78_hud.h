#ifndef _01FA78_HUD_H
#define _01FA78_HUD_H

/* Driver-points meter fill, ramped toward
 * var_runState_8c2285c4.driverPoints_0x0c over 20 frames. */
typedef struct {
    float displayedValue_0x00;
    float lastSample_0x04;
    float rampStep_0x08;
    float field_0x0c; /* set to 1.0f, never read */
} DriverPointsMeterState;

typedef struct {
    /* Unused. */
    int field_0x00;
    int field_0x04;
    int field_0x08;
    int field_0x0c;
    int driveMarkLatched_0x10;

    /* Sprite id of the last drive instruction passed; -1 before the first. */
    int driveMarkIcon_0x14;

    /* Frames since the instruction slot changed; also reset by
     * StopUpdateArrival_8c02ce48. */
    int blinkTimer_0x18;

    DriverPointsMeterState pointsMeter_0x1c;

    float engineRpm_0x2c;

    /* Unused. */
    int field_0x30;

    /* Set once the popup is shown, cleared when the bus flag drops. */
    int gearLatch_0x34;
    int laneLatch_0x38;
} HudState;

/* Never read. */
extern void* var_8c226434;
extern void* var_8c226438;
extern HudState var_hudState_8c22643c;

/* Also resets the HUD state for a new run. */
void HudInstall_8c02018c(void);

#endif // _01FA78_HUD_H
