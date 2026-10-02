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

/* The driver-comment popup on screen. blinkIconId_0x04 blinks on top of
 * marks 0x1e-0x50, the lane changes. */
typedef struct {
    int markSpriteId_0x00;
    int blinkIconId_0x04;
    int displayTimer_0x08;
    int blinkCounter_0x0c;
} HudMarkState;

/* njDrawPolygon vertex. */
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

/* HudVertex arrays as raw bytes.
 *
 * Green fill of the driver-points meter; drawHud_8c01fbac moves its right
 * edge. */
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

/* Half-transparent backdrop under the bottom HUD strip. */
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

/* Tachometer needle triangle, 3 local-space NJS_POINT3s. */
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

/* HH:MM:SS from a 30fps frame count. */
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

    /* One slot, by stopPhase_0x20: the drive instruction, a fixed
     * signal-reminder icon, or the next-stop icon. Hidden 2 frames in every 8
     * for its first 60 frames. */
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

    /* The bottom corner sits 16px left of the top, for the track's slanted
     * end. */
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

    /* The needle eases 200 rpm/frame toward 0 when off, 500 when cranking,
     * and targetRpm_0x2e8 (floored at 500) when running. */
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

/* In the original, a label drawHud_8c01fbac falls through into, with no
 * prologue of its own. It has no standalone test; drawHud_8c01fbac's tests
 * cover it. */
STATIC void drawSpeedAndTimers_8c01fe84(Sint32 speed) {
    Sint32 units, tens;

    if (speed < 0) {
        speed = -speed;
    }

    units = speed % 10;
    SpriteDraw_8c014f54(&var_busStopTexlist_8c1bc424, units, 320.0f, 420.0f, -1.21f);
    tens = speed / 10;
    SpriteDraw_8c014f54(&var_busStopTexlist_8c1bc424, tens, 308.0f, 420.0f, -1.21f);

    drawTimeDigits_8c01fa80(var_runState_8c2285c4.scheduleTime_0x14, 402.0f, 10);
    drawTimeDigits_8c01fa80(var_runState_8c2285c4.runClock_0x18, 423.0f, 20);

    SpriteDraw_8c014f54(&var_busStopTexlist_8c1bc424, 0x24, 0.0f, 0.0f, -1.23f);
}

/* Turns the map's marks (wipers, gear, headlights, lane change) into
 * driver-comment popups, ramps the points meter and queues drawHud_8c01fbac. */
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
        /* A fresh gear change skips the headlight and lane-change checks
         * for this frame. */
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

void HudInstall_8c02018c(void) {
    Task *installedTask;
    void *installedState;

    TaskInstall_8c014ae8(var_tasks_8c1ba5e8, updateTask_8c01ff48, &installedTask, &installedState, 0);

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
