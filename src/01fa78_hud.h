#ifndef _01FA78_HUD_H
#define _01FA78_HUD_H

/* Driver-points meter fill, ramped toward
 * var_runState_8c2285c4.driverPoints_0x0c over 20 frames.
 * field_0x0c is seeded to 1.0f and never read. */
typedef struct {
    float displayedValue_0x00;
    float lastSample_0x04;
    float rampStep_0x08;
    float field_0x0c;
} DriverPointsMeterState;

/* The HUD's per-frame state. The whole 60-byte block is one object: every
 * field is reached as a constant displacement off a single base register, and
 * the next address (var_tachoNeedleVerts_8c226478) is loaded as its own. */
typedef struct {
    /* HudSpawnTask_8c02018c zeroes field_0x00/0x04 and nothing reads them back;
     * field_0x08/0x0c are never touched at all. */
    int field_0x00;
    int field_0x04;
    int field_0x08;
    int field_0x0c;
    int driveMarkLatched_0x10;

    /* HUD sprite id for the map's drive instruction under the bus
     * (markDriveFlags_0x3b0's low 3 bits, + 0x1f), latched by
     * updateTask_8c01ff48 and only ever -1 before the run's
     * first marked cell. 02b464 and 02c884 read it as "the bus has passed
     * one". */
    int driveMarkIcon_0x14;

    /* Free-running frame counter gating the blink of that HUD slot; reset by
     * updateTask_8c01ff48 on a new instruction and by
     * StopUpdateArrival_8c02ce48 (02c884) on a stop-phase change. */
    int blinkTimer_0x18;

    DriverPointsMeterState pointsMeter_0x1c;

    float engineRpm_0x2c;

    /* Written (zeroed) by HudSpawnTask_8c02018c, never read. */
    int field_0x30;

    /* Gear-message / lane-change-message latch for updateTask_8c01ff48:
     * set once the corresponding driver-comment popup has been
     * staged, cleared when the bus-state bit returns to 0. */
    int gearLatch_0x34;
    int laneLatch_0x38;
} HudState;

/* Only ever written: GameInit_8c0134ec resets both to -1. */
extern void* var_8c226434;
extern void* var_8c226438;
extern HudState var_hudState_8c22643c;

/* Starts the in-drive HUD for a new run: installs its per-frame task
 * (updateTask_8c01ff48, which queues the renderer through the
 * fade-command queue) and clears the popup, instruction-slot and
 * driver-points-meter state in section B. */
void HudSpawnTask_8c02018c(void);

#endif // _01FA78_HUD_H
