/* @unit Cinematic */

#include <shinobi.h>

#include "includes.h" /* STATIC */
#include "strings.h"
#include "1ba1c8_globals.h"
#include "013ae8_route.h"
#include "014a9c_tasks.h"
#include "022464_render.h"
#include "024b4c_bus_camera.h"
#include "02a9fc_message_box.h"
#include "025870_cinematic.h"

/* ====================
 * Compiler Definitions
 * ====================
 */

/* The camera cue painted on the road, in the top byte of markDriveFlags_0x3b0. */
#define DEMO_CUE_MASK  0xff000000
#define DEMO_CUE_SHIFT 24

/* One caption glyph every two frames. */
#define CAPTION_FRAMES_PER_GLYPH_SHIFT 1

/* ====================
 * Type Declarations
 * ====================
 */

/* cameraKind_0x00 counts from BUS_CAMERA_DEMO_WORLD_POINT: kind 0's pos_0x04
 * is a world point, kinds 1 and 2 are bus-space offsets. */
typedef struct {
    int cameraKind_0x00;
    NJS_POINT3 pos_0x04;
    char *name_0x10;
} DemoShot;

typedef struct {
    int phase_0x00;
    int revealCount_0x04;
    int textboxHandle_0x08;
} DemoShotState;

/* ====================
 * Non-initialized Globals
 * ====================
 */

STATIC NJS_POINT3 var_demoShotPos_8c227e00;
/* DemoShot[] of the current route. */
STATIC int *var_demoShots_8c227e0c;
/* Cut to var_demoShotId_8c227dd4's shot without waiting for a cue. */
STATIC int var_demoShotRearm_8c227e10;

/* ====================
 * Initialized Globals
 * ====================
 */

/* Indexed by camera cue. */
STATIC DemoShot init_demoShotsShinjuku_8c045674[] = {
    {1, {4.0f, 0.5f, -4.0f}, MSG_DEMO_SHOT_SHINJUKU_0},
    {0, {4138.0f, 13.0f, 4452.0f}, MSG_DEMO_SHOT_SHINJUKU_1},
    {0, {3943.0f, 19.0f, 4413.0f}, MSG_DEMO_SHOT_SHINJUKU_2},
    {0, {3924.0f, 4.0f, 4298.0f}, MSG_DEMO_SHOT_SHINJUKU_3},
    {0, {3948.0f, 0.3f, 3718.0f}, MSG_DEMO_SHOT_SHINJUKU_4},
    {2, {-2.0f, 1.0f, -10.0f}, MSG_DEMO_SHOT_SHINJUKU_5},
    {1, {-2.0f, 1.0f, 6.0f}, MSG_DEMO_SHOT_SHINJUKU_6},
    {2, {3.0f, 0.8f, 10.0f}, MSG_DEMO_SHOT_SHINJUKU_7},
    {2, {-4.0f, 2.5f, -10.0f}, MSG_DEMO_SHOT_SHINJUKU_8},
    {2, {3.0f, 2.5f, -25.0f}, MSG_DEMO_SHOT_SHINJUKU_9},
    {1, {3.0f, 2.5f, -25.0f}, MSG_DEMO_SHOT_SHINJUKU_10},
    {2, {7.5f, 1.8f, 0.0f}, MSG_DEMO_SHOT_SHINJUKU_11},
    {2, {11.0f, 1.8f, 0.0f}, MSG_DEMO_SHOT_SHINJUKU_12},
    {0, {3474.0f, 1.0f, 3511.0f}, MSG_DEMO_SHOT_SHINJUKU_13},
    {0, {3438.0f, 40.0f, 3417.0f}, MSG_DEMO_SHOT_SHINJUKU_14},
    {2, {-3.0f, 2.0f, -6.0f}, MSG_DEMO_SHOT_SHINJUKU_15},
    {1, {-6.3f, 2.0f, -6.0f}, MSG_DEMO_SHOT_SHINJUKU_16},
    {0, {3154.0f, 4.8f, 3185.0f}, MSG_DEMO_SHOT_SHINJUKU_17},
    {2, {-5.0f, 3.0f, 9.0f}, MSG_DEMO_SHOT_SHINJUKU_18},
    {1, {-2.5f, 3.0f, -3.0f}, MSG_DEMO_SHOT_SHINJUKU_19},
    {2, {-4.0f, 15.5f, 20.0f}, MSG_DEMO_SHOT_SHINJUKU_20},
    {0, {2924.7f, 5.0f, 2583.0f}, MSG_DEMO_SHOT_SHINJUKU_21},
    {2, {-2.0f, 0.7f, 8.0f}, MSG_DEMO_SHOT_SHINJUKU_22},
    {0, {2764.0f, 7.0f, 2481.0f}, MSG_DEMO_SHOT_SHINJUKU_23},
    {2, {30.0f, 0.5f, 0.0f}, MSG_DEMO_SHOT_SHINJUKU_24},
    {2, {30.0f, 0.5f, 0.0f}, MSG_DEMO_SHOT_SHINJUKU_25},
    {0, {2607.0f, 1.0f, 2244.0f}, MSG_DEMO_SHOT_SHINJUKU_26},
    {0, {2565.0f, 0.5f, 2165.0f}, MSG_DEMO_SHOT_SHINJUKU_27},
    {2, {-5.0f, 0.5f, -3.0f}, MSG_DEMO_SHOT_SHINJUKU_28},
    {1, {-5.0f, 0.5f, -3.0f}, MSG_DEMO_SHOT_SHINJUKU_29},
    {0, {2400.0f, 6.0f, 1707.0f}, MSG_DEMO_SHOT_SHINJUKU_30},
    {2, {-15.0f, 5.5f, -7.0f}, MSG_DEMO_SHOT_SHINJUKU_31},
    {1, {-15.0f, 5.5f, -7.0f}, MSG_DEMO_SHOT_SHINJUKU_32},
    {2, {-6.0f, 1.0f, 0.0f}, MSG_DEMO_SHOT_SHINJUKU_33},
    {2, {-6.0f, 1.0f, -5.0f}, MSG_DEMO_SHOT_SHINJUKU_34},
    {1, {-5.0f, 1.0f, -5.0f}, MSG_DEMO_SHOT_SHINJUKU_35},
    {2, {-5.0f, 1.0f, 5.0f}, MSG_DEMO_SHOT_SHINJUKU_36},
    {0, {2416.0f, 35.0f, 770.0f}, MSG_DEMO_SHOT_SHINJUKU_37},
    {2, {0.0f, 7.0f, 25.0f}, MSG_DEMO_SHOT_SHINJUKU_38},
    {2, {0.0f, 7.0f, 25.0f}, MSG_DEMO_SHOT_SHINJUKU_39},
    {0, {2047.0f, 0.5f, 833.0f}, MSG_DEMO_SHOT_SHINJUKU_40},
    {2, {3.0f, 0.5f, 7.0f}, MSG_DEMO_SHOT_SHINJUKU_41},
    {2, {-3.0f, 0.5f, 7.0f}, MSG_DEMO_SHOT_SHINJUKU_42},
    {0, {1619.0f, 6.0f, 734.0f}, MSG_DEMO_SHOT_SHINJUKU_43},
    {0, {1448.0f, 7.0f, 694.0f}, MSG_DEMO_SHOT_SHINJUKU_44},
    {2, {0.0f, 2.5f, -20.0f}, MSG_DEMO_SHOT_SHINJUKU_45},
    {0, {1171.0f, 1.0f, 524.0f}, MSG_DEMO_SHOT_SHINJUKU_46},
    {0, {1171.0f, 1.0f, 524.0f}, MSG_DEMO_SHOT_SHINJUKU_47},
    {0, {1171.0f, 1.0f, 524.0f}, MSG_DEMO_SHOT_SHINJUKU_48},
    {0, {1024.0f, 1.0f, 465.0f}, MSG_DEMO_SHOT_SHINJUKU_49},
    {0, {1047.0f, 1.0f, 479.0f}, MSG_DEMO_SHOT_SHINJUKU_50},
    {1, {-3.0f, 0.8f, -3.0f}, MSG_DEMO_SHOT_SHINJUKU_51},
    {0, {947.0f, 5.0f, 398.0f}, MSG_DEMO_SHOT_SHINJUKU_52},
    {2, {-4.0f, 5.0f, -15.0f}, MSG_DEMO_SHOT_SHINJUKU_53},
    {1, {-4.0f, 5.0f, -15.0f}, MSG_DEMO_SHOT_SHINJUKU_54},
    {0, {658.0f, 5.0f, 300.0f}, MSG_DEMO_SHOT_SHINJUKU_55},
    {0, {658.0f, 5.0f, 300.0f}, MSG_DEMO_SHOT_SHINJUKU_56},
    {0, {652.0f, 1.5f, 210.0f}, MSG_DEMO_SHOT_SHINJUKU_57},
    {0, {584.0f, 0.0f, 171.0f}, MSG_DEMO_SHOT_SHINJUKU_58},
    {2, {-4.0f, 0.5f, 5.0f}, MSG_DEMO_SHOT_SHINJUKU_59},
    {2, {0.0f, 2.5f, 10.0f}, MSG_DEMO_SHOT_SHINJUKU_60},
    {0, {558.0f, 10.0f, 318.0f}, MSG_DEMO_SHOT_SHINJUKU_61},
    {0, {447.0f, 0.0f, 352.0f}, MSG_DEMO_SHOT_SHINJUKU_62},
};

STATIC DemoShot init_demoShotsWangan_8c045b60[] = {
    {0, {3792.0f, 1.2f, 2893.0f}, MSG_DEMO_SHOT_WANGAN_0},
    {0, {3947.0f, 13.0f, 2850.0f}, MSG_DEMO_SHOT_WANGAN_1},
    {2, {-3.0f, 3.0f, -8.0f}, MSG_DEMO_SHOT_WANGAN_2},
    {2, {3.0f, 3.0f, -8.0f}, MSG_DEMO_SHOT_WANGAN_3},
    {0, {4978.0f, 8.0f, 3180.0f}, MSG_DEMO_SHOT_WANGAN_4},
    {0, {4978.0f, 6.0f, 3180.0f}, MSG_DEMO_SHOT_WANGAN_5},
    {2, {3.0f, 0.5f, -7.0f}, MSG_DEMO_SHOT_WANGAN_6},
    {0, {4138.0f, 13.0f, 4452.0f}, MSG_DEMO_SHOT_WANGAN_7},
    {2, {3.0f, 13.0f, -5.0f}, MSG_DEMO_SHOT_WANGAN_8},
    {2, {-3.0f, 13.0f, -5.0f}, MSG_DEMO_SHOT_WANGAN_9},
    {0, {3598.0f, 2.0f, 3434.0f}, MSG_DEMO_SHOT_WANGAN_10},
    {2, {5.0f, 4.0f, -20.0f}, MSG_DEMO_SHOT_WANGAN_11},
    {2, {-3.0f, 0.3f, 2.0f}, MSG_DEMO_SHOT_WANGAN_12},
    {0, {3202.0f, 1.5f, 3670.0f}, MSG_DEMO_SHOT_WANGAN_13},
    {2, {-3.0f, 2.5f, 0.0f}, MSG_DEMO_SHOT_WANGAN_14},
    {0, {2660.0f, 1.0f, 4071.0f}, MSG_DEMO_SHOT_WANGAN_15},
    {2, {10.0f, 1.5f, 0.0f}, MSG_DEMO_SHOT_WANGAN_16},
    {2, {-3.0f, 0.5f, 10.0f}, MSG_DEMO_SHOT_WANGAN_17},
    {1, {-3.0f, 0.5f, 10.0f}, MSG_DEMO_SHOT_WANGAN_18},
    {0, {2742.0f, 31.5f, 4706.0f}, MSG_DEMO_SHOT_WANGAN_19},
    {2, {14.5f, 1.5f, 13.0f}, MSG_DEMO_SHOT_WANGAN_20},
    {1, {14.5f, 1.5f, 13.0f}, MSG_DEMO_SHOT_WANGAN_21},
    {0, {1898.0f, 2.0f, 3789.0f}, MSG_DEMO_SHOT_WANGAN_22},
    {0, {2100.0f, 25.0f, 3770.0f}, MSG_DEMO_SHOT_WANGAN_23},
    {0, {2116.0f, 0.5f, 3780.0f}, MSG_DEMO_SHOT_WANGAN_24},
    {0, {2097.0f, 0.8f, 3654.0f}, MSG_DEMO_SHOT_WANGAN_25},
    {0, {2348.0f, 12.0f, 3543.0f}, MSG_DEMO_SHOT_WANGAN_26},
    {2, {1.0f, 1.0f, -8.0f}, MSG_DEMO_SHOT_WANGAN_27},
    {2, {1.0f, 1.0f, -8.0f}, MSG_DEMO_SHOT_WANGAN_28},
    {2, {8.0f, 1.5f, -11.0f}, MSG_DEMO_SHOT_WANGAN_29},
    {1, {8.0f, 1.5f, -11.0f}, MSG_DEMO_SHOT_WANGAN_30},
    {2, {8.0f, 1.5f, 0.0f}, MSG_DEMO_SHOT_WANGAN_31},
    {1, {8.0f, 1.5f, 0.0f}, MSG_DEMO_SHOT_WANGAN_32},
    {0, {2779.0f, 13.0f, 3196.0f}, MSG_DEMO_SHOT_WANGAN_33},
    {0, {2779.0f, 9.8f, 3196.0f}, MSG_DEMO_SHOT_WANGAN_34},
    {2, {-2.1f, 0.5f, 7.0f}, MSG_DEMO_SHOT_WANGAN_35},
    {0, {2625.9f, 9.7f, 2858.6f}, MSG_DEMO_SHOT_WANGAN_36},
    {0, {1138.0f, 0.5f, 549.0f}, MSG_DEMO_SHOT_WANGAN_37},
    {2, {-3.0f, 0.5f, 6.0f}, MSG_DEMO_SHOT_WANGAN_38},
    {0, {1113.0f, 0.5f, 462.0f}, MSG_DEMO_SHOT_WANGAN_39},
    {2, {0.0f, 16.5f, 40.0f}, MSG_DEMO_SHOT_WANGAN_40},
    {0, {970.0f, 0.5f, 345.0f}, MSG_DEMO_SHOT_WANGAN_41},
    {2, {3.0f, 1.5f, -20.0f}, MSG_DEMO_SHOT_WANGAN_42},
    {1, {3.0f, 1.5f, -20.0f}, MSG_DEMO_SHOT_WANGAN_43},
    {0, {692.0f, 1.5f, 293.0f}, MSG_DEMO_SHOT_WANGAN_44},
};

STATIC DemoShot init_demoShotsOme_8c045ee4[] = {
    {0, {6992.0f, 0.7f, 6210.2f}, MSG_DEMO_SHOT_OME_0},
    {0, {7007.0f, 0.5f, 6225.0f}, MSG_DEMO_SHOT_OME_1},
    {2, {-3.0f, 0.7f, 8.0f}, MSG_DEMO_SHOT_OME_2},
    {0, {7190.0f, 6.4f, 6400.5f}, MSG_DEMO_SHOT_OME_3},
    {0, {7190.0f, 6.4f, 6400.5f}, MSG_DEMO_SHOT_OME_4},
    {2, {-3.0f, 0.7f, -10.0f}, MSG_DEMO_SHOT_OME_5},
    {1, {-3.0f, 0.7f, -10.0f}, MSG_DEMO_SHOT_OME_6},
    {0, {7262.0f, 0.5f, 6169.0f}, MSG_DEMO_SHOT_OME_7},
    {2, {-3.0f, 1.5f, 3.0f}, MSG_DEMO_SHOT_OME_8},
    {2, {5.0f, 1.5f, -10.0f}, MSG_DEMO_SHOT_OME_9},
    {1, {5.0f, 1.5f, -10.0f}, MSG_DEMO_SHOT_OME_10},
    {2, {0.0f, 3.0f, 10.0f}, MSG_DEMO_SHOT_OME_11},
    {0, {6854.0f, 2.0f, 5824.0f}, MSG_DEMO_SHOT_OME_12},
    {0, {6889.0f, 18.0f, 5736.0f}, MSG_DEMO_SHOT_OME_13},
    {0, {2707.0f, -1.0f, 1460.0f}, MSG_DEMO_SHOT_OME_14},
    {2, {-4.0f, 1.0f, -18.0f}, MSG_DEMO_SHOT_OME_15},
    {1, {-4.0f, 1.0f, -18.0f}, MSG_DEMO_SHOT_OME_16},
    {2, {3.0f, 0.5f, -3.0f}, MSG_DEMO_SHOT_OME_17},
    {0, {2283.0f, 4.0f, 1286.0f}, MSG_DEMO_SHOT_OME_18},
    {0, {2225.0f, 35.0f, 1169.0f}, MSG_DEMO_SHOT_OME_19},
    {0, {2135.0f, 8.0f, 1107.0f}, MSG_DEMO_SHOT_OME_20},
    {2, {0.0f, 4.0f, 10.0f}, MSG_DEMO_SHOT_OME_21},
    {0, {1992.0f, 12.0f, 797.0f}, MSG_DEMO_SHOT_OME_22},
};

char init_demoFirstShot_8c0460b0[] = {
    0x06, 0x13, 0x33, 0x00, 0x0A, 0x14, 0x1D, 0x25,
    0x00, 0x0E, 0x00, 0x00,
};

/* ====================
 * Functions
 * ====================
 */

void CinematicBoardingCamera_8c025870(void)
{
    njInitCamera(&var_cabinCamera_8c1bb984);
    njSetCameraAngle(&var_cabinCamera_8c1bb984, 12743);
    njSetCameraDepth(&var_cabinCamera_8c1bb984, -1.0f, -15.0f);
    njTranslateCameraPosition(&var_cabinCamera_8c1bb984, -0.2f, 1.8f, -1.6f);
    njPointCameraInterest(&var_cabinCamera_8c1bb984, 0.0f, 1.8f, 3.0f);
}

/* A world point's y is relative to the bus. Chase shots are resolved every
 * frame in CinematicUpdateCamera_8c025906 instead. */
STATIC void applyShotPosition_8c0258ba(void)
{
    switch (var_cameraMode_8c227d9c) {
    case BUS_CAMERA_DEMO_WORLD_POINT:
        var_busState_8c1bb9d0.posX_0x2fc = var_demoShotPos_8c227e00.x;
        var_busState_8c1bb9d0.posY_0x300 =
            var_busState_8c1bb9d0.posY_0x0f8 + var_demoShotPos_8c227e00.y;
        var_busState_8c1bb9d0.posZ_0x304 = var_demoShotPos_8c227e00.z;
        break;
    case BUS_CAMERA_DEMO_LOCAL_POINT:
        njCalcPoint(&var_busState_8c1bb9d0.worldMatrix_0x084,
                    &var_demoShotPos_8c227e00,
                    (NJS_POINT3 *)&var_busState_8c1bb9d0.posX_0x2fc);
        break;
    default:
        break;
    }
}

void CinematicUpdateCamera_8c025906(void)
{
    njInitCamera(&var_camera_8c1bb904);
    njSetCameraAngle(&var_camera_8c1bb904, 10194);
    njSetCameraDepth(&var_camera_8c1bb904, -1.0f, -300.0f);

    if (var_cameraMode_8c227d9c == BUS_CAMERA_DEMO_CHASE) {
        njCalcPoint(&var_busState_8c1bb9d0.worldMatrix_0x084,
                    &var_demoShotPos_8c227e00,
                    (NJS_POINT3 *)&var_busState_8c1bb9d0.posX_0x2fc);
    }

    if (var_cameraMode_8c227d9c == BUS_CAMERA_DEMO_WORLD_POINT ||
        var_cameraMode_8c227d9c == BUS_CAMERA_DEMO_LOCAL_POINT ||
        var_cameraMode_8c227d9c == BUS_CAMERA_DEMO_CHASE) {
        njTranslateCameraPosition(&var_camera_8c1bb904,
                                   var_busState_8c1bb9d0.posX_0x2fc,
                                   var_busState_8c1bb9d0.posY_0x300,
                                   var_busState_8c1bb9d0.posZ_0x304);
        njPointCameraInterest(&var_camera_8c1bb904,
                               var_busState_8c1bb9d0.posX_0x0f4,
                               var_busState_8c1bb9d0.posY_0x0f8,
                               var_busState_8c1bb9d0.posZ_0x0fc);
        var_busState_8c1bb9d0.moveDeltaX_0x308 =
            var_busState_8c1bb9d0.posX_0x2fc - var_busState_8c1bb9d0.posX_0x0f4;
        var_busState_8c1bb9d0.moveDeltaZ_0x310 =
            var_busState_8c1bb9d0.posZ_0x304 - var_busState_8c1bb9d0.posZ_0x0fc;
    }
}

/* Cuts to the shot of the camera cue under the bus and types its caption,
 * then waits for the cue to clear. */
STATIC void demoShotTask_8c0259e8(Task *task, DemoShotState *state)
{
    Uint32 cue;

    cue = var_busState_8c1bb9d0.markDriveFlags_0x3b0 & DEMO_CUE_MASK;

    switch (state->phase_0x00) {
    case 0:
        if (cue != 0 || var_demoShotRearm_8c227e10 != 0) {
            int shotId = (int)(cue >> DEMO_CUE_SHIFT);

            if (var_demoShotRearm_8c227e10 != 0) {
                shotId = var_demoShotId_8c227dd4;
                var_demoShotId_8c227dd4 = -1;
                var_demoShotRearm_8c227e10 = 0;
            }

            if (var_demoShotId_8c227dd4 != shotId) {
                DemoShot *shot = (DemoShot *)var_demoShots_8c227e0c + shotId;

                var_demoShotId_8c227dd4 = shotId;
                var_cameraMode_8c227d9c =
                    BUS_CAMERA_DEMO_WORLD_POINT + shot->cameraKind_0x00;
                var_demoShotPos_8c227e00 = shot->pos_0x04;

                if (shot->name_0x10[0] != 0) {
                    state->textboxHandle_0x08 = MessageBoxSwapFor_8c02aefc(shot->name_0x10);
                    state->revealCount_0x04 = 0;
                } else {
                    state->textboxHandle_0x08 = 0;
                }

                applyShotPosition_8c0258ba();
                state->phase_0x00 = 1;
            }
        }
        break;

    case 1:
        if (cue == 0) {
            state->phase_0x00 = 0;
        }
        break;
    }

    if (state->textboxHandle_0x08 != 0) {
        state->revealCount_0x04++;
        MessageBoxMenuTextboxText_8c02af1c(state->revealCount_0x04 >> CAPTION_FRAMES_PER_GLYPH_SHIFT);
    }

    RenderQueueDraw_8c0223ea(RENDER_LAYER_MAIN, (DrawFn)BusCameraDrawBusModel_8c024bb8, 0);
}

void CinematicStartTour_8c025af4(void)
{
    Task *task;
    DemoShotState *state;

    switch (var_route_8c18ad1c) {
    case ROUTE_SHINJUKU:
        var_demoShots_8c227e0c = (int *)init_demoShotsShinjuku_8c045674;
        break;
    case ROUTE_WANGAN:
        var_demoShots_8c227e0c = (int *)init_demoShotsWangan_8c045b60;
        break;
    case ROUTE_OME:
        var_demoShots_8c227e0c = (int *)init_demoShotsOme_8c045ee4;
        break;
    }

    TaskSpawn_8c014ae8(var_tasks_8c1ba5e8, &demoShotTask_8c0259e8, &task, (void **)&state, 0xc);
    state->phase_0x00 = 0;

    MessageBoxOpenTextbox_8c02ae3e(0x20, 0x180, -1.0f, 0x023E, 0x40, 0, 0, -1);
    var_demoShotRearm_8c227e10 = 1;
}

