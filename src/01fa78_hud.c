/* @unit Hud */
#include <shinobi.h>

#include "01fa78_hud.h"
#include "014a9c_tasks.h"
#include "014f54_sprite.h"
#include "015034_text.h"
#include "022464_render.h"
#include "028258_signal.h"
#include "02b464_grading.h"
#include "1ba1c8_globals.h"
#include "includes.h" /* STATIC */
#include "serial_debug.h"

/* ====================
 * Type Declarations
 * ====================
 */

/* The driver-comment popup currently on screen. showMark_8c01fa78
 * sets markSpriteId_0x00 and displayTimer_0x08 (frames left); blinkIconId_0x04
 * is a second sprite updateTask_8c01ff48 blinks on top of it, from the map
 * cell's scenePresetIds_0x3bc, and only while the mark id is in [0x1e, 0x50].
 * Both are drawn from var_markTexlist_8c1bc418. */
typedef struct {
    int markSpriteId_0x00;
    int blinkIconId_0x04;
    int displayTimer_0x08;
    int blinkCounter_0x0c;
} HudMarkState;

/* A screen-space vertex for njDrawPolygon: position plus an ARGB word. The
 * HUD's static quads are raw byte arrays of these. */
typedef struct {
    float x, y, z;
    Uint32 color;
} HudVertex;

/* ====================
 * Non-initialized Globals
 * ====================
 */

void* var_8c226434;
void* var_8c226438;
HudState var_hudState_8c22643c;
STATIC HudVertex var_tachoNeedleVerts_8c226478[3];
STATIC HudMarkState var_hudMark_8c2264a8;

/* ====================
 * Initialized Globals
 * ====================
 */

/* The HUD's static geometry, as raw HudVertex bytes (x,y,z,ARGB) to stay
 * byte-identical with the asm.
 *
 * Green fill of the driver-points meter: left edge pinned at x=38, right edge
 * (vertices 2/3, offsets 0x20/0x30) rewritten every frame by
 * drawHud_8c01fbac. The two right vertices are 16px apart, matching the
 * track's slanted end. */
STATIC Uint8 init_pointsMeterFill_8c045334[] = {
    0x00, 0x00, 0x18, 0x42, 0x00, 0x00, 0xD8, 0x43,
    0xFB, 0x91, 0x53, 0x3F, 0x00, 0xF4, 0x00, 0xFF,
    0x00, 0x00, 0x18, 0x42, 0x00, 0x00, 0xDE, 0x43,
    0xFB, 0x91, 0x53, 0x3F, 0x00, 0xF4, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x43,
    0xFB, 0x91, 0x53, 0x3F, 0x00, 0xF4, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDE, 0x43,
    0xFB, 0x91, 0x53, 0x3F, 0x00, 0xF4, 0x00, 0xFF,
};

/* The meter's red full-scale track, x=38..240, behind the fill. */
STATIC Uint8 init_pointsMeterTrack_8c045374[] = {
    0x00, 0x00, 0x18, 0x42, 0x00, 0x00, 0xD8, 0x43,
    0x4D, 0x21, 0x50, 0x3F, 0x00, 0x00, 0xB8, 0xFF,
    0x00, 0x00, 0x18, 0x42, 0x00, 0x00, 0xDE, 0x43,
    0x4D, 0x21, 0x50, 0x3F, 0x00, 0x00, 0xB8, 0xFF,
    0x00, 0x00, 0x70, 0x43, 0x00, 0x00, 0xD8, 0x43,
    0x4D, 0x21, 0x50, 0x3F, 0x00, 0x00, 0xB8, 0xFF,
    0x00, 0x00, 0x60, 0x43, 0x00, 0x00, 0xDE, 0x43,
    0x4D, 0x21, 0x50, 0x3F, 0x00, 0x00, 0xB8, 0xFF,
};

/* Half-transparent black backdrop under the whole bottom HUD strip; drawn
 * with njDrawPolygon's trans flag set. */
STATIC Uint8 init_hudPanel_8c0453b4[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xD4, 0x43,
    0x9D, 0x73, 0x4E, 0x3F, 0x00, 0x00, 0x00, 0x80,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xEF, 0x43,
    0x9D, 0x73, 0x4E, 0x3F, 0x00, 0x00, 0x00, 0x80,
    0x00, 0x00, 0x72, 0x43, 0x00, 0x80, 0xD4, 0x43,
    0x9D, 0x73, 0x4E, 0x3F, 0x00, 0x00, 0x00, 0x80,
    0x00, 0xC0, 0x1F, 0x44, 0x00, 0x80, 0xEF, 0x43,
    0x9D, 0x73, 0x4E, 0x3F, 0x00, 0x00, 0x00, 0x80,
    0x00, 0x80, 0x90, 0x43, 0x00, 0x00, 0xC1, 0x43,
    0x9D, 0x73, 0x4E, 0x3F, 0x00, 0x00, 0x00, 0x80,
    0x00, 0xC0, 0x1F, 0x44, 0x00, 0x00, 0xC1, 0x43,
    0x9D, 0x73, 0x4E, 0x3F, 0x00, 0x00, 0x00, 0x80,
};

/* 3 NJS_POINT3 local-space corners of the rotating tachometer needle triangle
 * (drawHud_8c01fbac), njCalcPoint'd through var_scratchMatrix_8c1bc46c into var_tachoNeedleVerts_8c226478. */
STATIC Uint8 init_tachoNeedle_8c045414[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0,
    0x08, 0xD6, 0x51, 0x3F, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x40, 0x08, 0xD6, 0x51, 0x3F,
    0x00, 0x00, 0x20, 0xC2, 0x00, 0x00, 0x00, 0x00,
    0x08, 0xD6, 0x51, 0x3F,
};

/* ====================
 * Forward Declarations
 * ====================
 */

STATIC void drawSpeedAndTimers_8c01fe84(Sint32 speed);

/* ====================
 * Functions
 * ====================
 */

STATIC void showMark_8c01fa78(int spriteId, int frames) {
    var_hudMark_8c2264a8.markSpriteId_0x00 = spriteId;
    var_hudMark_8c2264a8.displayTimer_0x08 = frames;
}

/* Draws a 6-digit HH:MM:SS readout from a 30fps frame count (108000 = 1h,
 * 1800 = 1min, 30 = 1sec), tens-then-units per field. */
STATIC void drawTimeDigits_8c01fa80(int frames, float y, int spriteBase) {
    int hours = frames / 108000;
    int minutes = (frames % 108000) / 1800;
    int seconds = (frames % 1800) / 30;

    SpriteDraw_8c014f54(&var_busStopTexlist_8c1bc424, spriteBase + hours % 10, 448.0f, y, -1.21f);
    SpriteDraw_8c014f54(&var_busStopTexlist_8c1bc424, spriteBase + hours / 10, 436.0f, y, -1.21f);
    SpriteDraw_8c014f54(&var_busStopTexlist_8c1bc424, spriteBase + minutes % 10, 477.0f, y, -1.21f);
    SpriteDraw_8c014f54(&var_busStopTexlist_8c1bc424, spriteBase + minutes / 10, 465.0f, y, -1.21f);
    SpriteDraw_8c014f54(&var_busStopTexlist_8c1bc424, spriteBase + seconds % 10, 506.0f, y, -1.21f);
    SpriteDraw_8c014f54(&var_busStopTexlist_8c1bc424, spriteBase + seconds / 10, 494.0f, y, -1.21f);
}

/* Per-frame in-drive HUD renderer: the driver-comment popup
 * (var_hudMark_8c2264a8, plus arg0 -- the traffic-signal code
 * updateTask_8c01ff48 stages through the fade-command queue), the shared
 * instruction/next-stop slot, the driver-points meter, the two blinker
 * indicators, the tachometer needle, and the speed readout plus the two
 * clocks drawn by the drawSpeedAndTimers_8c01fe84 tail. Installed both as a
 * DrawFn and as a TaskSpawn_8c014ae8 action, so it must keep this
 * exact signature. */
STATIC void drawHud_8c01fbac(int signalMarkId) {
    float barWidth, barWidthInner;
    Angle angle;
    Sint32 speed;

    if (var_playMode_8c1bb8d0 == PLAY_MODE_DEMO) {
        return;
    }

    if (var_hudMark_8c2264a8.displayTimer_0x08 != 0) {
        SpriteDraw_8c014f54(&var_markTexlist_8c1bc418, var_hudMark_8c2264a8.markSpriteId_0x00, 0.0f, 0.0f, -1.21f);
    }
    if (var_hudMark_8c2264a8.blinkIconId_0x04 != 0) {
        SpriteDraw_8c014f54(&var_markTexlist_8c1bc418, var_hudMark_8c2264a8.blinkIconId_0x04, 0.0f, 0.0f, -1.2f);
    }
    if (signalMarkId != 0) {
        SpriteDraw_8c014f54(&var_markTexlist_8c1bc418, signalMarkId, 0.0f, 0.0f, -1.21f);
    }

    /* One screen slot, three meanings, picked by var_runState_8c2285c4.stopPhase_0x20:
     * cruising shows the map's drive instruction, phase 1 a fixed
     * signal-reminder icon, phase 2 the next-stop icon. All blink on
     * var_hudState_8c22643c.blinkTimer_0x18 -- hidden 2 frames in every 8 for
     * the first 60, solid after. */
    if (var_runState_8c2285c4.stopPhase_0x20 == 0 || var_runState_8c2285c4.stopPhase_0x20 == 4) {
        if (var_hudState_8c22643c.driveMarkIcon_0x14 != -1
            && (60 < (Sint32)var_hudState_8c22643c.blinkTimer_0x18
                || (var_hudState_8c22643c.blinkTimer_0x18 & 6) != 0)) {
            SpriteDraw_8c014f54(&var_busStopTexlist_8c1bc424,
                var_hudState_8c22643c.driveMarkIcon_0x14, 0.0f, 0.0f, -1.2f);
        }
    } else if (var_runState_8c2285c4.stopPhase_0x20 == 1) {
        if (var_hudState_8c22643c.driveMarkIcon_0x14 != -1
            && (60 < (Sint32)var_hudState_8c22643c.blinkTimer_0x18
                || (var_hudState_8c22643c.blinkTimer_0x18 & 6) != 0)) {
            SpriteDraw_8c014f54(&var_busStopTexlist_8c1bc424, 0x1e, 0.0f, 0.0f, -1.2f);
        }
    } else if (var_runState_8c2285c4.stopPhase_0x20 == 2) {
        if (60 < (Sint32)var_hudState_8c22643c.blinkTimer_0x18
            || (var_hudState_8c22643c.blinkTimer_0x18 & 6) != 0) {
            SpriteDraw_8c014f54(&var_busStopTexlist_8c1bc424, 0x1f, 0.0f, 0.0f, -1.2f);
        }
    }

    /* Moving right edge (vertices 2/3) of the driver-points meter's fill
     * quad -- init_pointsMeterFill_8c045334's own vertex 0/1 (x=38.0) is the fixed left
     * edge; a second, narrower quad trails 16px behind, clamped to the
     * same left edge. */
    barWidth = (var_hudState_8c22643c.pointsMeter_0x1c.displayedValue_0x00 * 202.0f)
        / (float)var_runState_8c2285c4.driverPointsMax_0x10 + 38.0f;
    barWidthInner = barWidth - 16.0f;
    if (barWidthInner < 38.0f) {
        barWidthInner = 38.0f;
    }
    *(float *)&init_pointsMeterFill_8c045334[0x20] = barWidth;
    *(float *)&init_pointsMeterFill_8c045334[0x30] = barWidthInner;
    njDrawPolygon((NJS_POLYGON_VTX *)init_pointsMeterFill_8c045334, 4, 0);
    njDrawPolygon((NJS_POLYGON_VTX *)init_pointsMeterTrack_8c045374, 4, 0);
    njDrawPolygon((NJS_POLYGON_VTX *)init_hudPanel_8c0453b4, 6, 1);

    if (var_busState_8c1bb9d0.blinker_0x080 & 2) {
        SpriteDraw_8c014f54(&var_busStopTexlist_8c1bc424, 0x25, 0.0f, 0.0f, -1.21f);
    }
    if (var_busState_8c1bb9d0.blinker_0x080 & 4) {
        SpriteDraw_8c014f54(&var_busStopTexlist_8c1bc424, 0x26, 0.0f, 0.0f, -1.21f);
    }

    /* engineState_0x2e0 picks what the needle eases toward: off = 0,
     * cranking = 500, running = targetRpm_0x2e8 (floored at 500). 200 rpm per
     * frame, clamped on overshoot. */
    switch (var_busState_8c1bb9d0.engineState_0x2e0) {
    case 0:
        if (var_hudState_8c22643c.engineRpm_0x2c > 0.0f) {
            var_hudState_8c22643c.engineRpm_0x2c -= 200.0f;
            if (var_hudState_8c22643c.engineRpm_0x2c < 0.0f) {
                var_hudState_8c22643c.engineRpm_0x2c = 0.0f;
            }
        }
        break;

    case 1:
        if (var_hudState_8c22643c.engineRpm_0x2c > 500.0f) {
            var_hudState_8c22643c.engineRpm_0x2c -= 200.0f;
            if (var_hudState_8c22643c.engineRpm_0x2c >= 500.0f) break;
            var_hudState_8c22643c.engineRpm_0x2c = 500.0f;
        } else {
            var_hudState_8c22643c.engineRpm_0x2c += 200.0f;
            if (var_hudState_8c22643c.engineRpm_0x2c <= 500.0f) break;
            var_hudState_8c22643c.engineRpm_0x2c = 500.0f;
        }
        break;

    case 2: {
        float target = var_busState_8c1bb9d0.targetRpm_0x2e8;
        if (target > 500.0f) {
            if (target < var_hudState_8c22643c.engineRpm_0x2c) {
                var_hudState_8c22643c.engineRpm_0x2c -= 200.0f;
                if (target <= var_hudState_8c22643c.engineRpm_0x2c) break;
            } else {
                var_hudState_8c22643c.engineRpm_0x2c += 200.0f;
                if (var_hudState_8c22643c.engineRpm_0x2c <= target) break;
            }
            var_hudState_8c22643c.engineRpm_0x2c = target;
            break;
        }
        var_hudState_8c22643c.engineRpm_0x2c = 500.0f;
        break;
    }

    default:
        break;
    }

    njUnitMatrix(&var_scratchMatrix_8c1bc46c);
    njTranslate(&var_scratchMatrix_8c1bc46c, 320.0f, 436.0f, 0.0f);
    angle = ((Sint32)var_hudState_8c22643c.engineRpm_0x2c * 32768) / 6000;
    njRotateZ(&var_scratchMatrix_8c1bc46c, angle);
    njCalcPoint(&var_scratchMatrix_8c1bc46c, (NJS_POINT3 *)&init_tachoNeedle_8c045414[0], (NJS_POINT3 *)&var_tachoNeedleVerts_8c226478[0]);
    njCalcPoint(&var_scratchMatrix_8c1bc46c, (NJS_POINT3 *)&init_tachoNeedle_8c045414[12], (NJS_POINT3 *)&var_tachoNeedleVerts_8c226478[1]);
    njCalcPoint(&var_scratchMatrix_8c1bc46c, (NJS_POINT3 *)&init_tachoNeedle_8c045414[24], (NJS_POINT3 *)&var_tachoNeedleVerts_8c226478[2]);
    njDrawPolygon((NJS_POLYGON_VTX *)var_tachoNeedleVerts_8c226478, 3, 0);

    speed = (Sint32)((var_busState_8c1bb9d0.speed_0x27c * 108000.0f) / 1000.0f);
    drawSpeedAndTimers_8c01fe84(speed);
}

/* Tail of drawHud_8c01fbac's HUD render, split out because the original asm
 * falls through into this label from drawHud_8c01fbac with no other reference
 * to it anywhere in the tree (see drawHud_8c01fbac). Draws the 2-digit speed
 * value (its sign discarded) and the two elapsed-time readouts.
 *
 * No standalone test file: the asm label has no prologue of its own
 * (it inherits drawHud_8c01fbac's), so a direct call in the .src object reads
 * its argument from a stale R9 instead of R4 and its epilogue restores an
 * unestablished stack frame -- there is no way to invoke it in isolation
 * against that object. drawHud_8c01fbac's own tests exercise this function's
 * behavior in both objects: as a genuine call in the C object, and
 * (since the .src object can't have it mocked away either) via inline
 * assertions in that same file's assertSpeedTail() for the .src object. */
STATIC void drawSpeedAndTimers_8c01fe84(Sint32 speed) {
    Sint32 units, tens;

    if (speed < 0) {
        speed = -speed;
    }

    units = speed % 10;
    SpriteDraw_8c014f54(&var_busStopTexlist_8c1bc424, units, 320.0f, 420.0f, -1.21f);
    tens = speed / 10;
    SpriteDraw_8c014f54(&var_busStopTexlist_8c1bc424, tens, 308.0f, 420.0f, -1.21f);

    /* Timetable slot above, run clock below, each in its own digit set. */
    drawTimeDigits_8c01fa80(var_runState_8c2285c4.scheduleTime_0x14, 402.0f, 10);
    drawTimeDigits_8c01fa80(var_runState_8c2285c4.runClock_0x18, 423.0f, 20);

    SpriteDraw_8c014f54(&var_busStopTexlist_8c1bc424, 0x24, 0.0f, 0.0f, -1.23f);
}

/* Per-frame driver-comment popup logic: turns the instruction markers the
 * attribute grid hands BusTask_8c022bdc (markDriveFlags_0x3b0 -- wipers,
 * gear, headlights -- plus markAudioCue_0x3b8's lane change) into popups
 * staged via showMark_8c01fa78, ramps the driver-points meter display, and
 * queues drawHud_8c01fbac for this frame. */
STATIC void updateTask_8c01ff48() {
    unsigned int driveMark = var_busState_8c1bb9d0.markDriveFlags_0x3b0 & 7;
    unsigned int wiper;
    unsigned int gear;
    unsigned int lane;
    int signalMarkId = 0;

    if (var_hudState_8c22643c.driveMarkLatched_0x10 == 0) {
        if (driveMark != 0) {
            var_hudState_8c22643c.driveMarkIcon_0x14 = driveMark + 0x1f;
            var_hudState_8c22643c.driveMarkLatched_0x10 = 1;
            var_hudState_8c22643c.blinkTimer_0x18 = 0;
        }
    } else if (driveMark == 0) {
        var_hudState_8c22643c.driveMarkLatched_0x10 = 0;
    }

    wiper = var_busState_8c1bb9d0.markDriveFlags_0x3b0 & 0x30000;
    if (wiper != 0) {
        if (wiper == 0x20000) {
            showMark_8c01fa78(0x18, 300);
        } else if (wiper == 0x30000) {
            showMark_8c01fa78(0x19, 300);
        } else if (wiper == 0x10000) {
            showMark_8c01fa78(0x17, 300);
        }
    }

    gear = var_busState_8c1bb9d0.markDriveFlags_0x3b0 & 0xc0000;
    if (var_hudState_8c22643c.gearLatch_0x34 == 0 && gear != 0) {
        /* A fresh gear change hides the headlight and lane-change marks
         * for this frame; they only show once the latch clears again. */
        if (gear == 0x40000) {
            showMark_8c01fa78(0x1c, 0xb4);
        } else if (gear == 0x80000) {
            showMark_8c01fa78(0x1d, 0xb4);
        }
        var_hudState_8c22643c.gearLatch_0x34 = 1;
    } else {
        if (var_hudState_8c22643c.gearLatch_0x34 != 0 && gear == 0) {
            var_hudState_8c22643c.gearLatch_0x34 = 0;
        }

        if ((var_busState_8c1bb9d0.markDriveFlags_0x3b0 & 0x100) != 0) {
            showMark_8c01fa78(0x1b, 0xb4);
        }

        lane = var_busState_8c1bb9d0.markAudioCue_0x3b8 & 0xff0000;
        if (var_hudState_8c22643c.laneLatch_0x38 == 0) {
            if (lane != 0) {
                showMark_8c01fa78((short)(lane >> 0x10) + 0x1e, 0x78);
                var_hudState_8c22643c.laneLatch_0x38 = 1;
                var_hudMark_8c2264a8.blinkCounter_0x0c = 0;
            }
        } else if (lane == 0) {
            var_hudState_8c22643c.laneLatch_0x38 = 0;
        }
    }

    if (var_hudMark_8c2264a8.markSpriteId_0x00 < 0x1e || 0x50 < var_hudMark_8c2264a8.markSpriteId_0x00 || var_hudMark_8c2264a8.displayTimer_0x08 == 0) {
        var_hudMark_8c2264a8.blinkIconId_0x04 = 0;
    } else {
        var_hudMark_8c2264a8.blinkCounter_0x0c += 1;
        if ((var_hudMark_8c2264a8.blinkCounter_0x0c & 4) == 0) {
            var_hudMark_8c2264a8.blinkIconId_0x04 = 0;
        } else {
            var_hudMark_8c2264a8.blinkIconId_0x04 = (Uint8)var_busState_8c1bb9d0.scenePresetIds_0x3bc + 0x51;
        }
    }

    if ((var_busState_8c1bb9d0.markCueByte_0x3b4 & 0xff000000U) != 0) {
        int index = (Sint8)((unsigned int)var_busState_8c1bb9d0.markCueByte_0x3b4 >> 0x18);
        TrafficSignal *sig = SignalGet_8c0288b2(index);
        int frame = SignalGetFrame_8c028900(index);
        signalMarkId = frame + 1;

        if (sig->attached_0xbc[0] == 0) {
            if (sig->attached_0xbc[2] != 0) {
                signalMarkId = frame + 4;
                if (frame == 0) {
                    frame = sig->attached_0xbc[2]->frame_0x0c;
                    if (frame == 1) {
                        signalMarkId = 4 + 3;
                    }
                }
            }
        } else {
            signalMarkId = frame + 8;
            if (frame == 0) {
                frame = sig->attached_0xbc[0]->frame_0x0c;
                if (frame == 1) {
                    signalMarkId = 8 + 3;
                }
            }
        }
    }

    if (var_hudMark_8c2264a8.displayTimer_0x08 != 0) {
        var_hudMark_8c2264a8.displayTimer_0x08 -= 1;
    }
    var_hudState_8c22643c.blinkTimer_0x18 += 1;

    {
        float driverPointsF = (float)var_runState_8c2285c4.driverPoints_0x0c;
        if (driverPointsF != var_hudState_8c22643c.pointsMeter_0x1c.displayedValue_0x00) {
            if (driverPointsF != var_hudState_8c22643c.pointsMeter_0x1c.lastSample_0x04) {
                var_hudState_8c22643c.pointsMeter_0x1c.rampStep_0x08 =
                    (driverPointsF - var_hudState_8c22643c.pointsMeter_0x1c.lastSample_0x04) / 20.0f;
                var_hudState_8c22643c.pointsMeter_0x1c.lastSample_0x04 = driverPointsF;
            }
            var_hudState_8c22643c.pointsMeter_0x1c.displayedValue_0x00 +=
                var_hudState_8c22643c.pointsMeter_0x1c.rampStep_0x08;
            if ((var_hudState_8c22643c.pointsMeter_0x1c.rampStep_0x08 > 0.0f
                    && driverPointsF < var_hudState_8c22643c.pointsMeter_0x1c.displayedValue_0x00)
                || (var_hudState_8c22643c.pointsMeter_0x1c.rampStep_0x08 < 0.0f
                    && var_hudState_8c22643c.pointsMeter_0x1c.displayedValue_0x00 < driverPointsF)) {
                var_hudState_8c22643c.pointsMeter_0x1c.displayedValue_0x00 = driverPointsF;
            }
        }
    }

    RenderQueueDraw_8c0223ea(RENDER_LAYER_MAIN, drawHud_8c01fbac, signalMarkId);
}

/* Installs updateTask_8c01ff48 as a per-frame task and clears the popup,
 * instruction-slot and meter state for a fresh run. */
void HudSpawnTask_8c02018c(void) {
    Task *createdTask;
    void *createdState;

    TaskSpawn_8c014ae8(var_tasks_8c1ba5e8, updateTask_8c01ff48, &createdTask, &createdState, 0);

    var_hudState_8c22643c.field_0x00 = 0;
    var_hudState_8c22643c.field_0x04 = 0;
    var_hudState_8c22643c.driveMarkLatched_0x10 = 0;
    var_hudState_8c22643c.driveMarkIcon_0x14 = -1;
    var_hudState_8c22643c.blinkTimer_0x18 = 0;

    var_hudState_8c22643c.pointsMeter_0x1c.lastSample_0x04 = (float)var_runState_8c2285c4.driverPoints_0x0c;
    var_hudState_8c22643c.pointsMeter_0x1c.displayedValue_0x00 = (float)var_runState_8c2285c4.driverPoints_0x0c;
    var_hudState_8c22643c.pointsMeter_0x1c.field_0x0c = 1.0f;

    var_hudState_8c22643c.engineRpm_0x2c = 0.0f;
    var_hudState_8c22643c.field_0x30 = 0;
    var_hudState_8c22643c.gearLatch_0x34 = 0;
    var_hudState_8c22643c.laneLatch_0x38 = 0;

    var_tachoNeedleVerts_8c226478[0].color = 0xffff0000;
    var_tachoNeedleVerts_8c226478[1].color = 0xffff0000;
    var_tachoNeedleVerts_8c226478[2].color = 0xffff0000;
}
