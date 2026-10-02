/* @unit BusLine */
#include <shinobi.h>
#include <math.h> /* acosf */

#include "includes.h" /* TWO_PI */
#include "1ba1c8_globals.h"
#include "023938_bus_drive.h" /* LinePoint, LineBusSegment, LineBusNode */
#include "02412c_bus_line.h"
#include "023310_bus_init.h"

/* ====================
 * Functions
 * ====================
 */

int BusLineAdvance_8c02412c(void)
{
    LinePoint *point = (LinePoint *)var_busState_8c1bb9d0.currentLinePointPtr_0x2b8;
    float remaining = var_busState_8c1bb9d0.lineSegmentRemaining_0x2bc;
    float dx, dz, dist, dxN, dzN;
    float angle, angleUnits;
    int angleUnitsInt;

    for (;;) {
        if (point->len_0x00 > remaining) {
            break;
        }

        remaining -= point->len_0x00;
        point++;

        if (point->len_0x00 == 0.0f) {
            LineBusNode *nodes = var_lineNodes_8c227d88;
            LineBusSegment *segs = var_lineSegments_8c227d84;
            LineBusNode *node = &nodes[var_busState_8c1bb9d0.currentLineNodeIdx_0x33c];
            int nextIdx;

            /* At a junction, the turn signal picks the branch. */
            if (node->altNext_0x04[1] != LINE_NODE_NONE && var_busState_8c1bb9d0.signalSide_0x25c == 1) {
                nextIdx = node->altNext_0x04[1];
            } else if (node->altNext_0x04[2] != LINE_NODE_NONE && var_busState_8c1bb9d0.signalSide_0x25c == 2) {
                nextIdx = node->altNext_0x04[2];
            } else {
                nextIdx = node->altNext_0x04[0];
            }

            var_busState_8c1bb9d0.currentLineNodeIdx_0x33c = (Uint16)nextIdx;
            var_busState_8c1bb9d0.lineSegmentProgress_0x2c0 = remaining;
            point = segs[nextIdx].points_0x00;
        }
    }

    var_busState_8c1bb9d0.currentLinePointPtr_0x2b8 = (int)point;
    var_busState_8c1bb9d0.lineSegmentRemaining_0x2bc = remaining;

    var_busState_8c1bb9d0.laneTargetX_0x0ec = point->x_0x04 + remaining * point->dx_0x0c;
    var_busState_8c1bb9d0.laneTargetZ_0x0f0 = point->z_0x08 + remaining * point->dz_0x10;

    dx = var_busState_8c1bb9d0.posX_0x0f4 - var_busState_8c1bb9d0.laneTargetX_0x0ec;
    dz = var_busState_8c1bb9d0.posZ_0x0fc - var_busState_8c1bb9d0.laneTargetZ_0x0f0;
    dist = njSqrt(dx * dx + dz * dz);
    dxN = dx / dist;
    dzN = dz / dist;

    var_busState_8c1bb9d0.posX_0x0f4 = var_busState_8c1bb9d0.laneTargetX_0x0ec + dxN * var_busState_8c1bb9d0.laneOffset_0x2c4;
    var_busState_8c1bb9d0.posZ_0x0fc = var_busState_8c1bb9d0.laneTargetZ_0x0f0 + dzN * var_busState_8c1bb9d0.laneOffset_0x2c4;

    angle = acosf(dzN);
    angleUnits = angle * 65536.0f / TWO_PI;
    angleUnitsInt = (int)angleUnits;
    if (dxN <= 0.0f) {
        angleUnitsInt = -angleUnitsInt;
    }

    var_busState_8c1bb9d0.targetHeadingAngle_0x254 = angleUnitsInt;

    return 1;
}
