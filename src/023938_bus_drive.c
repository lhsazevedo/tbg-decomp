/* @unit BusDrive */
#include <shinobi.h>
#include "includes.h" /* STATIC */
#include "serial_debug.h"

#include <sg_sd.h>
#include "1ba1c8_globals.h"
#include "0100bc_sound.h" /* var_midiHandles_8c0fcd28 */
#include "0206f0_intersect.h" /* IntersectSegments_8c0206f0 */
#include "020b6c_ground_probe.h" /* GroundProbeInterpolateHeight_8c020f7e */
#include "0207d4_vec_xz.h" /* VecXZCross_8c0207fa, PointXZ */
#include "02081c_geom.h" /* GeomDistanceXZ_8c02081c */
#include "023938_bus_drive.h"
#include "023310_bus_init.h"
#include "02b464_grading.h"

/* ====================
 * Type Declarations
 * ====================
 */

/* groundProbeFn_0x2c8's real signature. */
typedef void (*GroundQueryFn)(float x, float y, float z, GroundQueryResult *out);

/* ====================
 * Functions
 * ====================
 */

void BusDriveEnterKnockback_8c023bce(void)
{
    var_busState_8c1bb9d0.rpmRampAngle_0x2e4 = 0;
    if (var_busState_8c1bb9d0.gear_0x2f4 != 5) {
        var_busState_8c1bb9d0.gear_0x2f4 = 0;
    }
    var_busState_8c1bb9d0.driveState_0x2b4 = 2;
}

STATIC void hitWall_8c023bea(void)
{
    if (var_busState_8c1bb9d0.speed_0x27c < 0.1388889f) {
        sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0x13, 0);
        var_busState_8c1bb9d0.speed_0x27c = 0.3f;
        var_wallHitBits_8c228660 |= 2;
    } else if (var_busState_8c1bb9d0.speed_0x27c < 0.2777778f) {
        sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0x14, 0);
        var_busState_8c1bb9d0.speed_0x27c = var_busState_8c1bb9d0.speed_0x27c / 2.0f + 0.3f;
        var_wallHitBits_8c228660 |= 4;
    } else {
        sdMidiPlay(var_midiHandles_8c0fcd28[0], 1, 0x15, 0);
        var_busState_8c1bb9d0.speed_0x27c = var_busState_8c1bb9d0.speed_0x27c / 2.0f + 0.3f;
        var_wallHitBits_8c228660 |= 6;
    }

    BusDriveEnterKnockback_8c023bce();
}

void BusDriveApplyGround_8c023cba(void)
{
    GroundQueryResult *gs = var_busState_8c1bb9d0.groundSamples_0x190;
    NJS_POINT3 *hist = var_busState_8c1bb9d0.posHistory_0x100;
    NJS_POINT3 *p = &var_groundQueryPoint_8c1bc460;
    int skipUpdate = 0;

    if (gs[6].attr_0x00 == 0 || gs[7].attr_0x00 == 0) {
        hitWall_8c023bea();
        var_busState_8c1bb9d0.dir_x2_0x2ac = var_busState_8c1bb9d0.headingX_0x230;
        var_busState_8c1bb9d0.dir_x_0x29c = var_busState_8c1bb9d0.headingX_0x230;
        var_busState_8c1bb9d0.dir_z2_0x2b0 = var_busState_8c1bb9d0.headingZ_0x238;
        var_busState_8c1bb9d0.dir_z_0x2a0 = var_busState_8c1bb9d0.headingZ_0x238;
    } else {
        float sign = 1.0f;
        int useDivide = 0;

        if (gs[4].attr_0x00 != 0 && gs[0].attr_0x00 != 0) {
            if (gs[2].attr_0x00 != 0 && gs[8].attr_0x00 != 0) {
                sign = -1.0f;
                if (gs[5].attr_0x00 == 0 || gs[1].attr_0x00 == 0) {
                    useDivide = 1;
                } else if (gs[3].attr_0x00 != 0 && gs[9].attr_0x00 != 0) {
                    skipUpdate = 1;
                }
            }
        } else {
            useDivide = 1;
        }

        if (!skipUpdate) {
            hitWall_8c023bea();
            p->x = sign;
            p->z = 0.0f;
            njUnitMatrix(&var_scratchMatrix_8c1bc46c);
            njRotateY(&var_scratchMatrix_8c1bc46c, var_busState_8c1bb9d0.ang_0x250);
            njCalcPoint(&var_scratchMatrix_8c1bc46c, p, p);
            var_busState_8c1bb9d0.dir_x_0x29c = p->x;
            var_busState_8c1bb9d0.dir_z_0x2a0 = p->z;
            if (useDivide) {
                var_busState_8c1bb9d0.dir_x2_0x2ac = p->x / 2.0f;
                var_busState_8c1bb9d0.dir_z2_0x2b0 = p->z / 2.0f;
            } else {
                var_busState_8c1bb9d0.dir_x2_0x2ac = p->x * 2.0f;
                var_busState_8c1bb9d0.dir_z2_0x2b0 = p->z * 2.0f;
            }
        }
    }

    GroundProbeInterpolateHeight_8c020f7e(&gs[0], (float *)&hist[2]);
    GroundProbeInterpolateHeight_8c020f7e(&gs[1], (float *)&hist[3]);
    var_busState_8c1bb9d0.posY_0x0f8 = (hist[2].y + hist[3].y) / 2.0f;

    GroundProbeInterpolateHeight_8c020f7e(&gs[2], (float *)&hist[4]);
    GroundProbeInterpolateHeight_8c020f7e(&gs[3], (float *)&hist[5]);
    hist[0].y = (hist[4].y + hist[5].y) / 2.0f;

    hist[7].y = var_busState_8c1bb9d0.posY_0x0f8;
    hist[6].y = var_busState_8c1bb9d0.posY_0x0f8;
    hist[9].y = var_busState_8c1bb9d0.posY_0x0f8;
    hist[8].y = var_busState_8c1bb9d0.posY_0x0f8;
    hist[11].y = hist[0].y;
    hist[10].y = hist[0].y;
}

void BusDriveSampleGround_8c023938(void)
{
    GroundQueryFn query = (GroundQueryFn)var_busState_8c1bb9d0.groundProbeFn_0x2c8;
    NJS_POINT3 *hist = var_busState_8c1bb9d0.posHistory_0x100;
    GroundQueryResult *gs = var_busState_8c1bb9d0.groundSamples_0x190;
    float origX = var_busState_8c1bb9d0.posX_0x0f4;
    float origZ = var_busState_8c1bb9d0.posZ_0x0fc;
    float dx = hist[0].x - origX;
    float dz = hist[0].z - origZ;
    float dist = njSqrt(dx * dx + dz * dz);
    float ndx = dx / dist;
    float ndz = dz / dist;
    float lat, lon;
    float baseX, baseZ;

    var_busState_8c1bb9d0.headingDirX_0x274 = ndx;
    var_busState_8c1bb9d0.headingDirZ_0x278 = ndz;
    var_busState_8c1bb9d0.headingX_0x230 = ndx * 4.9f;
    var_busState_8c1bb9d0.headingZ_0x238 = ndz * 4.9f;

    hist[0].x = origX + var_busState_8c1bb9d0.headingX_0x230;
    hist[0].z = origZ + var_busState_8c1bb9d0.headingZ_0x238;
    hist[1].x = origX + ndx * 8.0f;
    hist[1].z = origZ + ndz * 8.0f;

    lat = ndz * 1.25f;
    lon = ndx * 1.25f;

    hist[2].x = origX - lat;
    hist[2].z = origZ + lon;
    query(hist[2].x, hist[2].y, hist[2].z, &gs[0]);

    hist[3].x = origX + lat;
    hist[3].z = origZ - lon;
    query(hist[3].x, hist[3].y, hist[3].z, &gs[1]);

    hist[4].x = hist[1].x - lat;
    hist[4].z = hist[1].z + lon;
    query(hist[4].x, hist[4].y, hist[4].z, &gs[2]);

    hist[5].x = origX + lat;
    hist[5].z = origZ - lon;
    query(hist[5].x, hist[5].y, hist[5].z, &gs[3]);

    /* Original quirks: hist[10] repeats hist[4]'s point, and hist[5], built
     * from origX/origZ instead of hist[1], lands on hist[3]'s, so gs[3]
     * duplicates gs[1]. hist[11] is what hist[5] should have been. */
    hist[10].x = hist[1].x - lat;
    hist[10].z = hist[1].z + lon;
    query(hist[10].x, hist[10].y, hist[10].z, &gs[8]);

    hist[11].x = hist[1].x + lat;
    hist[11].z = hist[1].z - lon;
    query(hist[11].x, hist[11].y, hist[11].z, &gs[9]);

    baseX = origX - ndx * 2.6f;
    baseZ = origZ - ndz * 2.6f;

    hist[6].x = baseX - lat;
    hist[6].z = baseZ + lon;
    query(hist[6].x, var_busState_8c1bb9d0.posY_0x0f8, hist[6].z, &gs[4]);

    hist[7].x = baseX + lat;
    hist[7].z = baseZ - lon;
    query(hist[7].x, var_busState_8c1bb9d0.posY_0x0f8, hist[7].z, &gs[5]);

    lat /= 2.0f;
    lon /= 2.0f;

    hist[8].x = baseX - lat;
    hist[8].z = baseZ + lon;
    query(hist[8].x, hist[0].y, hist[8].z, &gs[6]);

    hist[9].x = baseX + lat;
    hist[9].z = baseZ - lon;
    query(hist[9].x, hist[0].y, hist[9].z, &gs[7]);
}

void BusDriveFindLaneTarget_8c023e7e(void)
{
    LineBusSegment *segs = var_lineSegments_8c227d84;
    LineBusNode *nodes = var_lineNodes_8c227d88;
    Uint16 idx;
    LinePoint *seg;
    LinePoint *segEnd;
    float remaining;
    float traveled;
    float cand[2];
    float side;
    int found;

    if (var_busState_8c1bb9d0.laneTargetSearchDone_0x334 != 0) {
        if (var_busState_8c1bb9d0.laneOffset_0x2c4 == 2.0f) {
            var_busState_8c1bb9d0.signalSide_0x25c = 0;
            var_busState_8c1bb9d0.mirror_0x268 = 0;
            var_busState_8c1bb9d0.laneTargetSearchDone_0x334 = 0;
        }
        return;
    }

    if (var_busState_8c1bb9d0.laneTargetSearchSide_0x338 == 2) {
        return;
    }

    if (var_busState_8c1bb9d0.laneTargetSearchSide_0x338 == 0) {
        idx = nodes[var_busState_8c1bb9d0.currentLineNodeIdx_0x33c].leftLane_0x00;
    } else {
        /* 1; the original leaves idx unset for any other value. */
        idx = nodes[var_busState_8c1bb9d0.currentLineNodeIdx_0x33c].rightLane_0x02;
    }

    if (idx != LINE_NODE_NONE) {
        seg = segs[idx].points_0x00;
        remaining = var_busState_8c1bb9d0.speed_0x27c * 128.0f
                  + segs[idx].length_0x04 * var_busState_8c1bb9d0.lineSegmentProgress_0x2c0
                  / segs[var_busState_8c1bb9d0.currentLineNodeIdx_0x33c].length_0x04;

        segEnd = seg;
        traveled = remaining;
        while (segEnd->len_0x00 <= remaining) {
            remaining -= segEnd->len_0x00;
            segEnd += 1;
            if (segEnd->len_0x00 == 0.0f) {
                idx = nodes[idx].fallbackNext_0x0a;
                if (idx == LINE_NODE_NONE) {
                    var_busState_8c1bb9d0.laneTargetSearchSide_0x338 = 2;
                    return;
                }
                segEnd = segs[idx].points_0x00;
                traveled = remaining;
            }
        }

        cand[0] = remaining * segEnd->dx_0x0c + segEnd->x_0x04;
        cand[1] = remaining * segEnd->dz_0x10 + segEnd->z_0x08;

        side = VecXZCross_8c0207fa((NJS_POINT3 *)&var_busState_8c1bb9d0.posX_0x0f4,
                                   (PointXZ *)&var_busState_8c1bb9d0.laneTargetX_0x0ec,
                                   (PointXZ *)cand);

        if (var_busState_8c1bb9d0.laneTargetSearchSide_0x338 == 0) {
            if (side > 0.0f) {
                var_busState_8c1bb9d0.laneTargetSearchSide_0x338 = 2;
                return;
            }
        } else if (var_busState_8c1bb9d0.laneTargetSearchSide_0x338 == 1 && side < 0.0f) {
            var_busState_8c1bb9d0.laneTargetSearchSide_0x338 = 2;
            return;
        }

        found = 0;
        while (!found) {
            if (seg == segEnd) {
                var_busState_8c1bb9d0.laneTargetX_0x0ec = cand[0];
                var_busState_8c1bb9d0.laneTargetZ_0x0f0 = cand[1];
                var_busState_8c1bb9d0.laneOffset_0x2c4 =
                    GeomDistanceXZ_8c02081c(&var_busState_8c1bb9d0.posX_0x0f4, cand);
                var_busState_8c1bb9d0.laneTargetSearchDone_0x334 = 1;
                var_busState_8c1bb9d0.currentLineNodeIdx_0x33c = idx;
                var_busState_8c1bb9d0.lineSegmentRemaining_0x2bc = remaining;
                var_busState_8c1bb9d0.lineSegmentProgress_0x2c0 = traveled;
                var_busState_8c1bb9d0.currentLinePointPtr_0x2b8 = (int)segEnd;
                return;
            }

            {
                LinePoint *next = seg + 1;
                Uint16 nextIdx = idx;
                if (next->len_0x00 == 0.0f) {
                    nextIdx = nodes[idx].fallbackNext_0x0a;
                    next = segs[nextIdx].points_0x00;
                }
                var_crossingIntersectPointZ_8c1bc45c = var_busState_8c1bb9d0.posZ_0x0fc;
                var_crossingIntersectPoint_8c1bc458 = var_busState_8c1bb9d0.posX_0x0f4;
                found = IntersectSegments_8c0206f0(&var_crossingIntersectPoint_8c1bc458, cand,
                                                    &seg->x_0x04, &next->x_0x04,
                                                    &var_crossingIntersectPoint_8c1bc458);
                idx = nextIdx;
                seg = next;
            }
        }

        /* Original bug: passes the handle array's address, not a handle. */
        sdMidiPlay((SDMIDI)(int)&var_midiHandles_8c0fcd28[0], 1, 2, 0);
    }

    var_busState_8c1bb9d0.laneTargetSearchSide_0x338 = 2;
}
