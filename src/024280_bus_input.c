/* @unit BusInput */
#include <shinobi.h>
#include <math.h>                 /* asinf */
#include "includes.h" /* TWO_PI, STATIC */

#include "serial_debug.h"
#include "024b4c_bus_camera.h"
#include "1ba1c8_globals.h"
#include "014a9c_tasks.h"         /* Task */
#include "026710_traffic.h"       /* TrafficEntry */
#include "0100bc_sound.h"         /* var_midiHandles_8c0fcd28 */
#include "024280_bus_input.h"
#include "02b464_grading.h"

/* ====================
 * Compiler Definitions
 * ====================
 */

#define PHYSICS_FPS 30

#define MAX_RPM 6000.0f

#define SPEED_PER_KMH (3.6f * PHYSICS_FPS)

/** Converts kilometers per hour to internal speed units. */
#define KMH_TO_SPEED(kmh) \
    ((kmh) / SPEED_PER_KMH)

#define TOP_SPEED_TO_RATIO(topSpeed) \
    (KMH_TO_SPEED(topSpeed) / MAX_RPM)

/* Converts gear ramp time in seconds to BAM units/frame. */
#define ACCEL_RATE_FROM_SECONDS(seconds) \
    (0x4000 / (PHYSICS_FPS * (seconds)))

/* =====================
 * Type Declarations
 * =====================
 */

typedef struct {
    /** How quickly the RPM ramps up, in BAM units/frame */
    int accelRate_0x00;

    /** How much speed the gear produces for a given engine RPM, in units/frame/RPM */
    float speedRatio_0x04;

    /** The speed at which the gear should be upshifted. */
    float upshiftSpeed_0x08;
} Gear;

/* =====================
 * Non-initialized Globals
 * =====================
 */

int var_brakePressPeak_8c227d8c;

/* =====================
 * Initialized Globals
 * =====================
 */

STATIC Gear init_gears_8c045638[5] = {
    {   /* [0] 1st gear */
        /* accelRate_0x00    */  ACCEL_RATE_FROM_SECONDS(1),
        /* speedRatio_0x04   */  TOP_SPEED_TO_RATIO(14.0f),
        /* upshiftSpeed_0x08 */  KMH_TO_SPEED(10.0f)
    },
    {   /* [1] 2nd gear */
        /* accelRate_0x00    */  ACCEL_RATE_FROM_SECONDS(4),
        /* speedRatio_0x04   */  TOP_SPEED_TO_RATIO(28.0f),
        /* upshiftSpeed_0x08 */  KMH_TO_SPEED(20.0f)
    },
    {   /* [2] 3rd gear */
        /* accelRate_0x00    */  ACCEL_RATE_FROM_SECONDS(8),
        /* speedRatio_0x04   */  TOP_SPEED_TO_RATIO(42.0f),
        /* upshiftSpeed_0x08 */  KMH_TO_SPEED(35.0f)
    },
    {   /* [3] 4th gear */
        /* accelRate_0x00    */  ACCEL_RATE_FROM_SECONDS(12),
        /* speedRatio_0x04   */  TOP_SPEED_TO_RATIO(56.0f),
        /* upshiftSpeed_0x08 */  KMH_TO_SPEED(50.0f)
    },
    {   /* [4] 5th gear */
        /* accelRate_0x00    */  ACCEL_RATE_FROM_SECONDS(16),
        /* speedRatio_0x04   */  TOP_SPEED_TO_RATIO(70.0f),
        /* upshiftSpeed_0x08 */  KMH_TO_SPEED(70.0f)
    },
};

/* =====================
 * Functions
 * =====================
 */

/* Called by BusTask_8c022bdc while the mirror view is up: holds every traffic
 * entry drawn in the mirror (mirrorVisible_0x268) that is heading roughly the
 * same way as the bus (within ~90 degrees) to 20 km/h below the bus's own
 * speed, so nothing closes on the mirror while the player is looking at it. */
void BusInputCapMirrorTraffic_8c024280(void)
{
    Task *task;
    float speedCap;

    speedCap = var_busState_8c1bb9d0.speed_0x27c - KMH_TO_SPEED(20.0f);
    if (speedCap < 0.0f) {
        speedCap = 0.0f;
    }

    for (task = var_tasks_8c1bac28; task->action != NULL; task++) {
        TrafficEntry *entry;
        int diff;

        if (task->action == (TaskAction)-1) {
            continue;
        }

        entry = (TrafficEntry *)task->state;
        if (entry->mirrorVisible_0x268 == 0) {
            continue;
        }

        diff = var_busState_8c1bb9d0.ang_0x250 - entry->heading_0x250;
        if (diff < 0) {
            diff = -diff;
        }
        if (diff < 0x4000) {
            entry->followSpeedCap_0x418 = speedCap;
        }
    }
}

/* Forward/reverse selector, manual transmission only: D-pad up puts the bus in
 * gear 0, down in reverse (5). The racing controller reaches it through
 * inputManualTask_8c012504's Y-button remap. */
STATIC void updateGearSelector_8c0242ce(void)
{
    if (var_driveMode_8c1bb8c8 != 0) {
        return;
    }

    if ((var_peripherals_8c1ba35c[0].press & PDD_DGT_KU) != 0) {
        var_busState_8c1bb9d0.gear_0x2f4 = 0;
    } else if ((var_peripherals_8c1ba35c[0].press & PDD_DGT_KD) != 0) {
        var_busState_8c1bb9d0.gear_0x2f4 = 5;
    }
}

/* Applies braking (the .l trigger) each frame. */
STATIC void applyBraking_8c024530(void)
{
    Uint8 prevDeadzone;
    float delta;
    float brakeAmount;
    int gear;
    float rpm;
    int angle;

    prevDeadzone = var_progress_8c1ba1cc.brakeSensitivity_0xd1;
    delta = (float)((int)var_peripherals_8c1ba35c[0].l - (int)prevDeadzone);

    brakeAmount = 0.002f + (var_busState_8c1bb9d0.speed_0x27c / 48.0f) *
        (delta / (255.0f - prevDeadzone)) * (delta / (255.0f - prevDeadzone));

    var_busState_8c1bb9d0.speed_0x27c -= brakeAmount;
    if (var_busState_8c1bb9d0.speed_0x27c < 0.0f) {
        var_busState_8c1bb9d0.speed_0x27c = 0.0f;
    }

    gear = var_busState_8c1bb9d0.gear_0x2f4;
    if (gear != 0 && init_gears_8c045638[gear - 1].upshiftSpeed_0x08 > var_busState_8c1bb9d0.speed_0x27c) {
        gear -= 1;
        var_busState_8c1bb9d0.gear_0x2f4 = gear;
    }

    rpm = var_busState_8c1bb9d0.speed_0x27c / init_gears_8c045638[gear].speedRatio_0x04;
    var_busState_8c1bb9d0.targetRpm_0x2e8 = rpm;

    angle = (int)((asinf(rpm / MAX_RPM) * 65536.0f) / TWO_PI);
    var_busState_8c1bb9d0.rpmRampAngle_0x2e4 = angle;

    /* Running average of brakeAmount; gradeFrame_8c02bcd8 (02b464) docks
     * INSTR_HARD_BRAKE once it passes 0.01. */
    var_runState_8c2285c4.brakeAverage_0x90 += brakeAmount;
    var_runState_8c2285c4.brakeAverage_0x90 /= 2.0f;
}

/* Throttle handler: while the .r trigger is pushed past its deadzone and at
 * least as far as the engine has already wound up (var_busState_8c1bb9d0.rpmRampAngle_0x2e4
 * is this same rpmRampAngle_0x2e4, reached through its own section B symbol),
 * winds the ramp on at the current gear's accelRate_0x00 and upshifts through
 * the gear table; otherwise coasts and downshifts. */
STATIC void applyThrottle_8c024320(void)
{
    Uint16 trigger;
    Uint8 deadzone;
    float delta;
    int step;
    int gear;

    trigger = var_peripherals_8c1ba35c[0].r;
    deadzone = var_progress_8c1ba1cc.accelSensitivity_0xd0;
    delta = (float)((int)trigger - (int)deadzone);
    step = (int)((delta / (255.0f - deadzone)) * 16384.0f);

    if (trigger > deadzone && step >= var_busState_8c1bb9d0.rpmRampAngle_0x2e4) {
        gear = var_busState_8c1bb9d0.gear_0x2f4;

        var_busState_8c1bb9d0.rpmRampAngle_0x2e4 += init_gears_8c045638[gear].accelRate_0x00;
        if (var_busState_8c1bb9d0.rpmRampAngle_0x2e4 > step) {
            var_busState_8c1bb9d0.rpmRampAngle_0x2e4 = step;
        }

        var_busState_8c1bb9d0.targetRpm_0x2e8 =
            njSin(var_busState_8c1bb9d0.rpmRampAngle_0x2e4) * MAX_RPM;
        var_busState_8c1bb9d0.speed_0x27c =
            var_busState_8c1bb9d0.targetRpm_0x2e8 * init_gears_8c045638[gear].speedRatio_0x04;

        if (gear >= 4) {
            /* Top gear: no upshift left, just clamp. */
            if (var_busState_8c1bb9d0.speed_0x27c > KMH_TO_SPEED(70.0f)) {
                var_busState_8c1bb9d0.speed_0x27c = KMH_TO_SPEED(70.0f);
            }
            return;
        }

        if (var_busState_8c1bb9d0.speed_0x27c <= init_gears_8c045638[gear].upshiftSpeed_0x08) {
            return;
        }

        /* Shift-cue note: the mirror camera modes (>= 2) use a different note than the rest. */
        sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, (var_cameraMode_8c227d9c >= BUS_CAMERA_THIRD_PERSON_NEAR) ? 0x25 : 0x26, 0);

        if (gear == 0) {
            /* Arms gradeFrame_8c02bcd8's rapid-acceleration check (02b464). */
            var_runState_8c2285c4.firstUpshift_0x88 = 1;
        }
        var_busState_8c1bb9d0.gear_0x2f4 = gear + 1;
    } else {
        var_busState_8c1bb9d0.speed_0x27c -= 0.00025f;
        if (var_busState_8c1bb9d0.speed_0x27c < 0.0f) {
            var_busState_8c1bb9d0.speed_0x27c = 0.0f;
        }

        gear = var_busState_8c1bb9d0.gear_0x2f4;
        if (gear != 0 && init_gears_8c045638[gear - 1].upshiftSpeed_0x08 > var_busState_8c1bb9d0.speed_0x27c) {
            /* Same shift cue as the upshift above. */
            sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, (var_cameraMode_8c227d9c >= BUS_CAMERA_THIRD_PERSON_NEAR) ? 0x25 : 0x26, 0);
            gear -= 1;
            var_busState_8c1bb9d0.gear_0x2f4 = gear;
        }
    }

    gear = var_busState_8c1bb9d0.gear_0x2f4;
    var_busState_8c1bb9d0.targetRpm_0x2e8 =
        var_busState_8c1bb9d0.speed_0x27c / init_gears_8c045638[gear].speedRatio_0x04;
    var_busState_8c1bb9d0.rpmRampAngle_0x2e4 =
        (int)((asinf(var_busState_8c1bb9d0.targetRpm_0x2e8 / MAX_RPM) * 65536.0f) / TWO_PI);
}

/* Plays the brake pedal's SFX and tracks its press intensity in
 * var_brakePressPeak_8c227d8c, ratcheting it up to (never down from) the scaled travel
 * distance while the .l trigger keeps moving further past the saved
 * brakeSensitivity_0xd1 deadzone. */
STATIC void applyBrakingSfx_8c024606(void)
{
    Uint16 trigger;
    Uint8 prevDeadzone;

    trigger = var_peripherals_8c1ba35c[0].l;
    prevDeadzone = var_progress_8c1ba1cc.brakeSensitivity_0xd1;

    if (trigger > prevDeadzone) {
        int delta = trigger - prevDeadzone;
        int target = (delta * 255) / (255 - prevDeadzone);

        if (var_brakePressPeak_8c227d8c < target) {
            var_brakePressPeak_8c227d8c = target;
        }
    } else if (var_brakePressPeak_8c227d8c != 0) {
        /* Released: play the low note (0x27) if the peak press was past
         * the halfway point (0x80), else the high note (0x28). */
        int note = (var_brakePressPeak_8c227d8c >= 0x80) ? 0x27 : 0x28;
        sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, note, 0);
        var_brakePressPeak_8c227d8c = 0;
    }
}

/* Per-frame driving dispatcher, called by BusTask_8c022bdc while driving.
 * Plays the brake SFX, drives BusState.engineState_0x2e0 through its
 * off/starting/running states, handles the two turn-signal buttons (which
 * double as the mirror-view selector, and in mapped-route steering mode
 * also arm the lane-target search), and, in direct steering mode, runs the
 * steering-wheel force-feedback ramp. */
void BusInputUpdate_8c0246b2(void)
{
    const PDS_PERIPHERAL *pad = &var_peripherals_8c1ba35c[0];
    Uint16 brakeTrigger, throttleTrigger;
    Uint8 brakeDeadzone, throttleDeadzone;
    int mode;
    Uint32 press;

    applyBrakingSfx_8c024606();

    brakeTrigger = pad->l;
    brakeDeadzone = var_progress_8c1ba1cc.brakeSensitivity_0xd1;
    throttleTrigger = pad->r;
    throttleDeadzone = var_progress_8c1ba1cc.accelSensitivity_0xd0;

    mode = var_busState_8c1bb9d0.engineState_0x2e0;
    switch (mode) {
    case 0: /* engine off */
        if (brakeTrigger > brakeDeadzone) {
            var_busState_8c1bb9d0.blinker_0x080 |= 1;
        } else if (throttleTrigger > (Uint16)(throttleDeadzone / 2)) {
            var_busState_8c1bb9d0.idleFrameCounter_0x2ec = 0;
            var_busState_8c1bb9d0.engineState_0x2e0 = 1;
            VibStart_8c010f7a(0);
        }
        updateGearSelector_8c0242ce();
        break;

    case 1: { /* starting -- the 30-frame crank before the engine runs */
        int counter;

        if (brakeTrigger > brakeDeadzone) {
            var_busState_8c1bb9d0.blinker_0x080 |= 1;
        }

        counter = var_busState_8c1bb9d0.idleFrameCounter_0x2ec;
        var_busState_8c1bb9d0.idleFrameCounter_0x2ec = counter + 1;

        if (counter > 30) {
            /* Advance to running only if the throttle is STILL held (the
             * press that got here in the first place) and the brake isn't;
             * anything else -- throttle let go, or the brake now pressed --
             * aborts back to off. */
            if (throttleTrigger > throttleDeadzone && !(brakeTrigger > brakeDeadzone)) {
                var_busState_8c1bb9d0.engineState_0x2e0 = 2;
                var_busState_8c1bb9d0.idleFrameCounter_0x2ec = 0;
                var_busState_8c1bb9d0.field_0x2f0 = 0;
            } else {
                var_busState_8c1bb9d0.idleFrameCounter_0x2ec = 0;
                var_busState_8c1bb9d0.engineState_0x2e0 = 0;
                if (var_vibport_8c1ba354 != (Uint32)-1) {
                    pdVibMxStop(var_vibport_8c1ba354);
                }
            }
        }
        updateGearSelector_8c0242ce();
        break;
    }

    case 2: /* running */
        if (var_busState_8c1bb9d0.gear_0x2f4 == 5) {
            /* Reverse: drive speed_0x27c directly instead of going through
             * applyThrottle_8c024320/applyBraking_8c024530's forward-gear tables. */
            if (brakeTrigger > brakeDeadzone) {
                float ratio = (float)(brakeTrigger - brakeDeadzone) /
                    (255.0f - brakeDeadzone);
                var_busState_8c1bb9d0.speed_0x27c += ratio * 0.01f;
                if (var_busState_8c1bb9d0.speed_0x27c > 0.0f) {
                    var_busState_8c1bb9d0.speed_0x27c = 0.0f;
                }
                var_busState_8c1bb9d0.blinker_0x080 |= 1;
            } else if (throttleTrigger > throttleDeadzone) {
                float ratio = (float)(throttleTrigger - throttleDeadzone) /
                    (255.0f - throttleDeadzone);
                float target = -(ratio * KMH_TO_SPEED(20.0f));

                if (var_busState_8c1bb9d0.speed_0x27c > target) {
                    var_busState_8c1bb9d0.speed_0x27c += -0.0015432097716256976f;
                    if (var_busState_8c1bb9d0.speed_0x27c < target) {
                        var_busState_8c1bb9d0.speed_0x27c = target;
                    }
                } else {
                    var_busState_8c1bb9d0.speed_0x27c -= -0.0015432097716256976f;
                    if (var_busState_8c1bb9d0.speed_0x27c > target) {
                        var_busState_8c1bb9d0.speed_0x27c = target;
                    }
                }
            } else {
                var_busState_8c1bb9d0.speed_0x27c += 0.001f;
                if (var_busState_8c1bb9d0.speed_0x27c > 0.0f) {
                    var_busState_8c1bb9d0.speed_0x27c = 0.0f;
                }
            }

            var_busState_8c1bb9d0.targetRpm_0x2e8 =
                -(var_busState_8c1bb9d0.speed_0x27c * 16384.0f);
        } else {
            /* var_runState_8c2285c4.field_0x80 counts frames under way and nothing reads it
             * back -- a dead store. */
            if (var_busState_8c1bb9d0.speed_0x27c == 0.0f) {
                var_runState_8c2285c4.field_0x80 = 0;
            } else {
                var_runState_8c2285c4.field_0x80 += 1;
            }

            if (brakeTrigger > brakeDeadzone) {
                applyBraking_8c024530();
                var_busState_8c1bb9d0.blinker_0x080 |= 1;
            } else {
                applyThrottle_8c024320();
                /* Not braking: applyBraking_8c024530's running average
                 * decays straight to 0 rather than halving. */
                var_runState_8c2285c4.brakeAverage_0x90 = 0.0f;
            }
        }

        if (var_busState_8c1bb9d0.speed_0x27c == 0.0f) {
            updateGearSelector_8c0242ce();
            if (var_busState_8c1bb9d0.idleFrameCounter_0x2ec >= 30) {
                var_busState_8c1bb9d0.engineState_0x2e0 = 1;
                var_busState_8c1bb9d0.idleFrameCounter_0x2ec = 0;
                VibStart_8c010f7a(0);
            } else {
                var_busState_8c1bb9d0.idleFrameCounter_0x2ec += 1;
            }
        } else {
            var_busState_8c1bb9d0.idleFrameCounter_0x2ec = 0;
        }
        break;

    default:
        break;
    }

    press = pad->press;

    /* press bits 0x400/0x2 = left/right turn-signal buttons; signalSide_0x25c
     * is the driver's latched signal intent (0 off, 1 left, 2 right) and
     * doubles as the mirror-view selector -- toggling it sets mirror_0x268
     * between its 0/1/2 modes, gated by a var_runState_8c2285c4.lane_0x6c check against a
     * sentinel (0x10000000) or against var_runState_8c2285c4.field_0x70[0]. */
    if (var_driveMode_8c1bb8c8 != 0) {
        var_busState_8c1bb9d0.laneTargetSearchSide_0x338 = 2;
        if (var_busState_8c1bb9d0.laneTargetSearchDone_0x334 != 0) {
            return;
        }

        if ((press & PDD_DGT_TX) != 0) {
            switch (var_busState_8c1bb9d0.signalSide_0x25c) {
            case 0:
                var_busState_8c1bb9d0.signalSide_0x25c = 1;
                if (var_runState_8c2285c4.lane_0x6c != 0x10000000) {
                    var_busState_8c1bb9d0.mirror_0x268 = MIRROR_LEFT;
                }
                break;
            case 1:
                var_busState_8c1bb9d0.laneTargetSearchSide_0x338 = 0;
                break;
            case 2:
                var_busState_8c1bb9d0.signalSide_0x25c = 0;
                var_busState_8c1bb9d0.mirror_0x268 = MIRROR_NONE;
                break;
            default:
                break;
            }
        } else if ((press & PDD_DGT_TB) != 0) {
            switch (var_busState_8c1bb9d0.signalSide_0x25c) {
            case 0:
                var_busState_8c1bb9d0.signalSide_0x25c = 2;
                if ((var_runState_8c2285c4.lane_0x6c & ~1) != (var_runState_8c2285c4.field_0x70[0] << 4)) {
                    var_busState_8c1bb9d0.mirror_0x268 = MIRROR_RIGHT;
                }
                break;
            case 1:
                var_busState_8c1bb9d0.signalSide_0x25c = 0;
                var_busState_8c1bb9d0.mirror_0x268 = MIRROR_NONE;
                break;
            case 2:
                var_busState_8c1bb9d0.laneTargetSearchSide_0x338 = 1;
                break;
            default:
                break;
            }
        }
        return;
    }

    /* Direct steering mode: same two mirror buttons, but case 1 (held) just
     * clears back to 0 on either button, and laneTargetSearchSide_0x338 is never touched
     * here -- then the steering force-feedback ramp always runs below. */
    if ((press & PDD_DGT_TX) != 0) {
        switch (var_busState_8c1bb9d0.signalSide_0x25c) {
        case 0:
            var_busState_8c1bb9d0.signalSide_0x25c = 1;
            if (var_runState_8c2285c4.lane_0x6c != 0x10000000) {
                var_busState_8c1bb9d0.mirror_0x268 = MIRROR_LEFT;
            }
            break;
        case 1:
        case 2:
            var_busState_8c1bb9d0.signalSide_0x25c = 0;
            var_busState_8c1bb9d0.mirror_0x268 = MIRROR_NONE;
            break;
        default:
            break;
        }
    } else if ((press & PDD_DGT_TB) != 0) {
        switch (var_busState_8c1bb9d0.signalSide_0x25c) {
        case 0:
            var_busState_8c1bb9d0.signalSide_0x25c = 2;
            if ((var_runState_8c2285c4.lane_0x6c & ~1) != (var_runState_8c2285c4.field_0x70[0] << 4)) {
                var_busState_8c1bb9d0.mirror_0x268 = MIRROR_RIGHT;
            }
            break;
        case 1:
        case 2:
            var_busState_8c1bb9d0.signalSide_0x25c = 0;
            var_busState_8c1bb9d0.mirror_0x268 = MIRROR_NONE;
            break;
        default:
            break;
        }
    }

    /* Steering-wheel force-feedback ramp: eases ang_0x258 toward targetAngle
     * (BAM units, from the dead-zoned analog axis x1). Moving further from
     * center steps by a per-frame float ~80.89 or a plain int 80 -- an
     * asymmetry in the original code; moving back toward or past center
     * steps by a flat 182 (target keeps the current angle's sign) or 364
     * (target is zero or the opposite sign, for a faster return to center). */
    {
        Sint16 x1 = pad->x1;
        int dev;
        int targetAngle;
        int cur;

        if (-x1 > 0) {
            /* steering left */
            dev = -x1 - 8;
            if (dev > 0) {
                if (dev >= 127) {
                    targetAngle = 0x2aaa;
                } else {
                    targetAngle = (int)((float)(dev * 60 / 127) * 65536.0f / 360.0f);
                }
            } else {
                targetAngle = 0;
            }
        } else {
            /* steering right (or centered) */
            dev = -x1 + 8;
            if (dev < 0) {
                if (dev > -127) {
                    targetAngle = (int)((float)(dev * 60 / 127) * 65536.0f / 360.0f);
                } else {
                    targetAngle = -0x2aaa;
                }
            } else {
                targetAngle = 0;
            }
        }

        cur = var_busState_8c1bb9d0.ang_0x258;
        if (cur > 0) {
            if (cur >= targetAngle) {
                cur += (targetAngle * cur > 0) ? -182 : -364;
                var_busState_8c1bb9d0.ang_0x258 = cur;
                if (cur < targetAngle) {
                    var_busState_8c1bb9d0.ang_0x258 = targetAngle;
                }
            } else {
                cur = (int)((float)cur + 80.888885f);
                var_busState_8c1bb9d0.ang_0x258 = cur;
                if (cur > targetAngle) {
                    var_busState_8c1bb9d0.ang_0x258 = targetAngle;
                }
            }
        } else {
            if (cur <= targetAngle) {
                cur += (targetAngle * cur > 0) ? 182 : 364;
                var_busState_8c1bb9d0.ang_0x258 = cur;
                if (cur > targetAngle) {
                    var_busState_8c1bb9d0.ang_0x258 = targetAngle;
                }
            } else {
                cur -= 80;
                var_busState_8c1bb9d0.ang_0x258 = cur;
                if (cur < targetAngle) {
                    var_busState_8c1bb9d0.ang_0x258 = targetAngle;
                }
            }
        }
    }
}
