/* @unit BusDraw */

#include <shinobi.h>

#include "includes.h" /* STATIC */
#include "013ae8_route.h"
#include "1ba1c8_globals.h"
#include "026710_traffic.h"      /* TrafficEntry */
#include "020914_ground_query.h" /* GroundQueryResult */
#include "020b6c_ground_probe.h" /* GroundProbeTrackPolygonAtHeight_8c021290, GroundProbeInterpolateHeight_8c020f7e */
#include "020594_vehicle_model.h" /* VehicleModelPlace_8c020594 */
#include "022464_render.h"         /* RenderQueueDraw2_8c022420 */
#include "028258_signal.h" /* TrafficSignal */
#include "027958_bus_draw.h"

/* ====================
 * Compiler Definitions
 * ====================
 */

/* Every node this unit switches is authored with the same eval flags, so
 * showing and hiding one only moves NJD_EVAL_HIDE. */
#define EVAL_SHOWN  (NJD_EVAL_UNIT_POS | NJD_EVAL_UNIT_ANG | NJD_EVAL_UNIT_SCL | \
                     NJD_EVAL_BREAK | NJD_EVAL_ZXY_ANG)
#define EVAL_HIDDEN (EVAL_SHOWN | NJD_EVAL_HIDE)

#define LAMP_COUNT ((int)(sizeof(var_busState_8c1bb9d0.blinkerLights_0x02c) / \
                          sizeof(var_busState_8c1bb9d0.blinkerLights_0x02c[0])))

/* One world unit of travel is one full wheel revolution. */
#define WHEEL_TURNS_PER_UNIT 65536.0f

/* Suspension lean, in BAMS (0x10000 = 360 degrees): roll 1 degree per frame
 * up to 4, pitch 0.75 up to 3. */
#define ROLL_STEP_MAX   0x0b6
#define ROLL_MAX        0x2d8
#define PITCH_STEP_MAX  0x089
#define PITCH_MAX       0x222
#define PITCH_PER_ACCEL 45000.0f

/* ====================
 * Forward Declarations
 * ====================
 */

STATIC void drawAhead_8c027a88(int entityArg, int lod);
STATIC void drawMirror_8c027bac(int entityArg, int lod);

/* ====================
 * Functions
 * ====================
 */

void BusDrawUpdateModels_8c027958(BusState *bus)
{
    int i;

    bus->bodyNode_0x018->ang[0] = bus->pitchAngle_0x078;
    bus->bodyNode_0x018->ang[2] = bus->rollAngle_0x07c;
    bus->frontWheelA_0x01c->ang[0] = bus->distanceTraveled_0x070;
    bus->frontWheelA_0x01c->ang[1] = bus->steerAngle_0x074;
    bus->frontWheelB_0x020->ang[0] = bus->distanceTraveled_0x070;
    bus->frontWheelB_0x020->ang[1] = bus->steerAngle_0x074;
    bus->rearWheelA_0x024->ang[0] = bus->distanceTraveled_0x070;

    for (i = 0; i < LAMP_COUNT; i++) {
        bus->blinkerLights_0x02c[i]->evalflags =
            (bus->blinker_0x080 & (1 << i)) ? EVAL_SHOWN : EVAL_HIDDEN;
    }

    switch (bus->typeCode_0x000) {
    /* Vehicle types with no extra nodes bound. */
    case 0x0:
    case 0x2:
    case 0x4:
    case 0x6:
    case 0x8:
    case 0x12:
        break;

    case 0x14:
    case 0x16:
        bus->rearWheelB_0x028->ang[0] = bus->distanceTraveled_0x070;
        bus->turnLampB_0x044->evalflags =
            (bus->blinker_0x080 & 0x40) ? EVAL_SHOWN : EVAL_HIDDEN;
        break;

    case 0xe:
    case 0x10:
        bus->rearWheelB_0x028->ang[0] = bus->distanceTraveled_0x070;
        break;

    case 0x1c:
    case 0x1e:
        bus->turnLampA_0x040->evalflags =
            (bus->blinker_0x080 & 0x20) ? EVAL_SHOWN : EVAL_HIDDEN;
        break;

    case 0x1a:
        bus->turnLampC_0x048->evalflags =
            (bus->blinker_0x080 & 0x40) ? EVAL_SHOWN : EVAL_HIDDEN;
        break;

    default:
        break;
    }
}

STATIC void drawAhead_8c027a88(int entityArg, int lod)
{
    TrafficEntry *entity = (TrafficEntry *)entityArg;

    njMultiMatrix(0, &entity->worldMatrix_0x84);

    if (lod == 0) {
        njCnkSetSimpleLightIntensity(entity->simpleLightIntensity0_0x0c4,
                                     entity->simpleLightIntensity1_0x0c8);
        njCnkSetSimpleLightColor(entity->simpleLightColorR_0x0cc,
                                 entity->simpleLightColorG_0x0d0,
                                 entity->simpleLightColorB_0x0d4);
        BusDrawUpdateModels_8c027958((BusState *)entity);
        njSetTexture(entity->texlistLarge_0x04);
        njCnkSimpleDrawObject((NJS_CNK_OBJECT *)entity->modelLarge_0x0c);
        njControl3D(NJD_CONTROL_3D_MODEL_CLIP | NJD_CONTROL_3D_SHADOW |
                    NJD_CONTROL_3D_TRANS_MODIFIER);
        njCnkModDrawObject((NJS_CNK_OBJECT *)entity->shadowModel_0x14);
        njControl3D(NJD_CONTROL_3D_MODEL_CLIP);
    } else if (var_timeOfDay_8c18ad20 == TIME_OF_DAY_NIGHT) {
        njCnkSetSimpleLightIntensity(entity->simpleLightIntensity0_0x0c4,
                                     entity->simpleLightIntensity1_0x0c8);
        njCnkSetSimpleLightColor(entity->simpleLightColorR_0x0cc,
                                 entity->simpleLightColorG_0x0d0,
                                 entity->simpleLightColorB_0x0d4);
        njSetTexture(entity->texlistSmall_0x08);
        njCnkSimpleDrawObject((NJS_CNK_OBJECT *)entity->modelSmall_0x10);
    } else {
        njCnkSetEasyLightIntensity(entity->easyLightIntensity0_0x0d8,
                                   entity->easyLightIntensity1_0x0dc);
        njCnkSetEasyLightColor(entity->easyLightColorR_0x0e0,
                               entity->easyLightColorG_0x0e4,
                               entity->easyLightColorB_0x0e8);
        njSetTexture(entity->texlistSmall_0x08);
        njCnkEasyDrawObject((NJS_CNK_OBJECT *)entity->modelSmall_0x10);
    }
}

STATIC void drawMirror_8c027bac(int entityArg, int lod)
{
    TrafficEntry *entity = (TrafficEntry *)entityArg;

    njMultiMatrix(0, &entity->worldMatrix_0x84);

    if (lod == 0) {
        njCnkSetSimpleLightIntensity(entity->simpleLightIntensity0_0x0c4,
                                     entity->simpleLightIntensity1_0x0c8);
        njCnkSetSimpleLightColor(entity->simpleLightColorR_0x0cc,
                                 entity->simpleLightColorG_0x0d0,
                                 entity->simpleLightColorB_0x0d4);
        BusDrawUpdateModels_8c027958((BusState *)entity);
        njSetTexture(entity->texlistLarge_0x04);
        njCnkSimpleDrawObject((NJS_CNK_OBJECT *)entity->modelLarge_0x0c);
    } else {
        njCnkSetEasyLightIntensity(entity->easyLightIntensity0_0x0d8,
                                   entity->easyLightIntensity1_0x0dc);
        njCnkSetEasyLightColor(entity->easyLightColorR_0x0e0,
                               entity->easyLightColorG_0x0e4,
                               entity->easyLightColorB_0x0e8);
        njSetTexture(entity->texlistSmall_0x08);
        njCnkEasyDrawObject((NJS_CNK_OBJECT *)entity->modelSmall_0x10);
    }
}

void BusDrawPlaceEntity_8c027c3c(TrafficEntry *entity, float accel)
{
    int registered = 0;
    float dx, dz, dist;

    /* 0.174 and 0.966 are cos 80 and cos 15 degrees. */
    dx = var_busState_8c1bb9d0.posX_0x2fc - entity->posX_0xf4;
    dz = var_busState_8c1bb9d0.posZ_0x304 - entity->posZ_0xfc;
    dist = njSqrt(dx * dx + dz * dz);
    if (dist < 200.0f &&
        0.174f <= (var_busState_8c1bb9d0.moveDeltaX_0x308 * dx +
                   dz * var_busState_8c1bb9d0.moveDeltaZ_0x310) /
                  (var_busState_8c1bb9d0.moveDeltaMagnitude_0x314 * dist)) {
        RenderQueueDraw2_8c022420(RENDER_LAYER_MAIN, drawAhead_8c027a88, (int)entity, 55.0f <= dist);
        registered = 1;
    }

    dx = entity->posX_0xf4 - var_busState_8c1bb9d0.mirrorWorldOffsetX_0x318;
    dz = entity->posZ_0xfc - var_busState_8c1bb9d0.mirrorWorldOffsetZ_0x320;
    dist = njSqrt(dx * dx + dz * dz);
    if (dist < 50.0f &&
        0.966f < (var_busState_8c1bb9d0.mirrorDirX_0x324 * dx +
                  dz * var_busState_8c1bb9d0.mirrorDirZ_0x32c) /
                 (var_busState_8c1bb9d0.mirrorDist_0x330 * dist)) {
        int farLod;

        if (28.0f <= dist) {
            farLod = 1;
        } else {
            farLod = 0;
            if (15.0f < dist) {
                entity->mirrorVisible_0x268 = 1;
            }
        }
        RenderQueueDraw2_8c022420(RENDER_LAYER_MIRROR, drawMirror_8c027bac, (int)entity, farLod);
        registered = 1;
    }

    /* An entity on the polygon tracker re-aligns every frame. */
    if (entity->probeFn_0x2c8 == GroundProbeTrackPolygonAtHeight_8c021290) {
        registered = 1;
        entity->groundAligned_0x494 = 0;
    }

    /* An undrawn entity near the bus keeps its physics. */
    if (registered || entity->busDistance_0x490 < 85.333336f) {
        int prev, delta;
        int steerAngle;

        entity->distanceTraveled_0x070 -=
            (int)(entity->speed_0x27c * WHEEL_TURNS_PER_UNIT);

        steerAngle = entity->headingAlt_0x254 - entity->heading_0x250;
        entity->headingDelta_0x258 = steerAngle;
        entity->steerAngle_0x074 = steerAngle;

        prev = entity->rollAngle_0x07c;
        delta = (int)-((float)steerAngle * entity->speed_0x27c) - prev;
        if (delta < 0) {
            if (delta < -ROLL_STEP_MAX) {
                delta = -ROLL_STEP_MAX;
            }
            delta += prev;
            if (delta < -ROLL_MAX) {
                delta = -ROLL_MAX;
            }
        } else {
            if (delta > ROLL_STEP_MAX) {
                delta = ROLL_STEP_MAX;
            }
            delta += prev;
            if (delta > ROLL_MAX) {
                delta = ROLL_MAX;
            }
        }
        entity->rollAngle_0x07c = delta;

        prev = entity->pitchAngle_0x078;
        delta = (int)(accel * PITCH_PER_ACCEL) - prev;
        if (delta < 0) {
            if (delta < -PITCH_STEP_MAX) {
                delta = -PITCH_STEP_MAX;
            }
            delta += prev;
            if (delta < -PITCH_MAX) {
                delta = -PITCH_MAX;
            }
        } else {
            if (delta > PITCH_STEP_MAX) {
                delta = PITCH_STEP_MAX;
            }
            delta += prev;
            if (delta > PITCH_MAX) {
                delta = PITCH_MAX;
            }
        }
        entity->pitchAngle_0x078 = delta;

        if (entity->groundAligned_0x494 == 0 || entity->speed_0x27c != 0.0f) {
            GroundProbeFn probe = entity->probeFn_0x2c8;
            GroundQueryResult *probeA = &entity->groundProbe_0x190[0];
            GroundQueryResult *probeB = &entity->groundProbe_0x190[1];
            GroundQueryResult *probeC = &entity->groundProbe_0x190[2];

            /* An entity being pushed out of a collision is probed against the
             * bus grid instead of its own. */
            if (entity->driveState_0x2b4 == 1) {
                var_activeGroundGrid_8c2264d4 = var_currentCourse_8c1bb868.atariBus_0x04;
            }

            probe(entity->probeSideAX_0x118, entity->probeSideAY_0x11c,
                  entity->probeSideAZ_0x120, probeA);
            probe(entity->probeSideBX_0x124, entity->probeSideBY_0x128,
                  entity->probeSideBZ_0x12c, probeB);
            probe(entity->frontPointX_0x100, entity->frontPointY_0x104,
                  entity->frontPointZ_0x108, probeC);

            GroundProbeInterpolateHeight_8c020f7e(probeA, &entity->probeSideAX_0x118);
            GroundProbeInterpolateHeight_8c020f7e(probeB, &entity->probeSideBX_0x124);
            entity->posY_0xf8 =
                (entity->probeSideAY_0x11c + entity->probeSideBY_0x128) / 2.0f;
            GroundProbeInterpolateHeight_8c020f7e(probeC, &entity->frontPointX_0x100);

            if (entity->driveState_0x2b4 == 1) {
                var_activeGroundGrid_8c2264d4 = var_currentCourse_8c1bb868.atariCpu_0x18;
            }

            VehicleModelPlace_8c020594(&entity->worldMatrix_0x84, (BusState *)entity);
            entity->groundAligned_0x494 = 1;
            return;
        }
    }

    entity->groundAligned_0x494 = 0;
}

void BusDrawSignal_8c0281ac(int objArg, int matrixArg)
{
    TrafficSignal *obj = (TrafficSignal *)objArg;
    NJS_MATRIX *matrix = (NJS_MATRIX *)matrixArg;

    switch (obj->frame_0x0c) {
    case 1:
        obj->lamps_0x10[0]->evalflags = EVAL_SHOWN;
        obj->lamps_0x10[1]->evalflags = EVAL_HIDDEN;
        obj->lamps_0x10[2]->evalflags = EVAL_HIDDEN;
        break;
    case 2:
        obj->lamps_0x10[0]->evalflags = EVAL_HIDDEN;
        obj->lamps_0x10[1]->evalflags = EVAL_SHOWN;
        obj->lamps_0x10[2]->evalflags = EVAL_HIDDEN;
        break;
    case 0:
        obj->lamps_0x10[0]->evalflags = EVAL_HIDDEN;
        obj->lamps_0x10[1]->evalflags = EVAL_HIDDEN;
        obj->lamps_0x10[2]->evalflags = EVAL_SHOWN;
        break;
    default:
        break;
    }

    njMultiMatrix(0, matrix);
    njSetTexture(obj->tlist_0xb4);
    njCnkSimpleDrawObject((NJS_CNK_OBJECT *)obj->model_0xb8);
}

void BusDrawFadeLights_8c028022(BusState *bus)
{
    bus->blinker_0x080 |= 0x18;

    switch (bus->lightFadeState_0x2d8) {
    case 0: /* dark, waiting for the gate */
        if (bus->lightFadeGate_0x2dc != 0) {
            bus->lightFadeState_0x2d8 = 1;
        }
        break;

    case 1: /* fading up */
        bus->lightCoeffRow_0x0c4[0] += var_nightLightIntensityStep_8c1bbda0[0];
        if (bus->lightCoeffRow_0x0c4[0] >= var_nightLightIntensityOn_8c1bbdb0[0]) {
            bus->lightCoeffRow_0x0c4[0] = var_nightLightIntensityOn_8c1bbdb0[0];
            bus->lightCoeffRow_0x0c4[1] = var_nightLightIntensityOn_8c1bbdb0[1];
            bus->lightCoeffRow_0x0c4[2] = var_nightLightColorOn_8c1bbdd0[0];
            bus->lightCoeffRow_0x0c4[3] = var_nightLightColorOn_8c1bbdd0[1];
            bus->lightCoeffRow_0x0c4[4] = var_nightLightColorOn_8c1bbdd0[2];
            bus->lightFadeState_0x2d8 = 2;
        } else {
            bus->lightCoeffRow_0x0c4[1] += var_nightLightIntensityStep_8c1bbda0[1];
            bus->lightCoeffRow_0x0c4[2] += var_nightLightColorStep_8c1bbdb8[0];
            bus->lightCoeffRow_0x0c4[3] += var_nightLightColorStep_8c1bbdb8[1];
            bus->lightCoeffRow_0x0c4[4] += var_nightLightColorStep_8c1bbdb8[2];
        }
        break;

    case 2: /* lit, waiting for the gate to drop */
        if (bus->lightFadeGate_0x2dc == 0) {
            bus->lightFadeState_0x2d8 = 3;
        }
        break;

    case 3: /* fading down */
        bus->lightCoeffRow_0x0c4[0] -= var_nightLightIntensityStep_8c1bbda0[0];
        if (bus->lightCoeffRow_0x0c4[0] <= var_nightLightIntensityOff_8c1bbda8[0]) {
            bus->lightCoeffRow_0x0c4[0] = var_nightLightIntensityOff_8c1bbda8[0];
            bus->lightCoeffRow_0x0c4[1] = var_nightLightIntensityOff_8c1bbda8[1];
            bus->lightCoeffRow_0x0c4[2] = var_nightLightColorOff_8c1bbdc4[0];
            bus->lightCoeffRow_0x0c4[3] = var_nightLightColorOff_8c1bbdc4[1];
            bus->lightCoeffRow_0x0c4[4] = var_nightLightColorOff_8c1bbdc4[2];
            bus->lightFadeState_0x2d8 = 0;
        } else {
            bus->lightCoeffRow_0x0c4[1] -= var_nightLightIntensityStep_8c1bbda0[1];
            bus->lightCoeffRow_0x0c4[2] -= var_nightLightColorStep_8c1bbdb8[0];
            bus->lightCoeffRow_0x0c4[3] -= var_nightLightColorStep_8c1bbdb8[1];
            bus->lightCoeffRow_0x0c4[4] -= var_nightLightColorStep_8c1bbdb8[2];
        }
        break;
    }
}

void BusDrawSignalAttachment_8c028206(int objArg, int matrixArg)
{
    TrafficSignal *obj = (TrafficSignal *)objArg;
    NJS_MATRIX *matrix = (NJS_MATRIX *)matrixArg;

    if (obj->drawA_0xc8 == 0) {
        obj->lamps_0x10[0]->evalflags |= NJD_EVAL_HIDE;
    } else {
        obj->lamps_0x10[0]->evalflags &= ~NJD_EVAL_HIDE;
    }

    njMultiMatrix(0, matrix);
    njSetTexture(obj->tlist_0xb4);
    njCnkSimpleDrawObject((NJS_CNK_OBJECT *)obj->model_0xb8);
}
