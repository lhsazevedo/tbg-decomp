#ifndef _023938_BUS_DRIVE_H
#define _023938_BUS_DRIVE_H

/* One point on a route line segment (var_lineSegments_8c227d84 entries point into an
 * array of these). */
typedef struct {
    float len_0x00;  /* remaining distance to the next point */
    float x_0x04;    /* next point's world x/z */
    float z_0x08;
    float dx_0x0c;   /* unit direction of the segment leading to it */
    float dz_0x10;
} LinePoint;

/* var_lineSegments_8c227d84 entry: a route line's point list plus its total length. */
typedef struct {
    LinePoint *points_0x00;
    float length_0x04;
} LineBusSegment;

/* var_lineNodes_8c227d88 entry, 0xc bytes/6 ushorts. */
typedef struct {
    Uint16 leftLane_0x00;    /* read by BusDriveFindLaneTarget_8c023e7e */
    Uint16 rightLane_0x02;   /* read by BusDriveFindLaneTarget_8c023e7e */
    /* [0] is the default (used by 02412c), [1] is the next segment when
     * signalSide_0x25c == 1, [2] the one when it is 2 (02412c reads the same
     * field through var_busState_8c1bb9d0.signalSide_0x25c) */
    Uint16 altNext_0x04[3];
    Uint16 fallbackNext_0x0a;
} LineBusNode;

/* LineBusNode link with no target */
#define LINE_NODE_NONE 0xffff

/* Resets the bus's drivetrain to idle after a collision or a braking-sound
 * update; puts driveState_0x2b4 into the knockback state (2). */
void BusDriveEnterKnockback_8c023bce(void);

/* Computes 10 corner/lookahead ground-sample points around the bus and
 * queries each through the ground-query callback in groundProbeFn_0x2c8, filling
 * groundSamples_0x190; also reseeds posHistory_0x100[0]/[1] and the heading
 * unit vector headingDirX_0x274/0x278. Called by placeBus_8c023310/
 * BusInitStart_8c023610 (023310_bus_init) and BusTask_8c022bdc (022bdc). */
void BusDriveSampleGround_8c023938(void);

/* Walks the current route line segment forward by the bus's per-frame move
 * distance to find the next lane-crossing point, confirming it against the
 * crossing segment via IntersectSegments_8c0206f0. Called once per frame by
 * BusTask_8c022bdc (022bdc). */
void BusDriveFindLaneTarget_8c023e7e(void);

/* Rebuilds the bus's steering-correction direction from the corner
 * ground-probe results BusDriveSampleGround_8c023938 fills into groundSamples_0x190, then
 * averages two pairs of those probes' interpolated heights into
 * posY_0x0f8/posHistory_0x100[0].y. Called by BusInitStart_8c023610 (023310_bus_init)
 * and BusTask_8c022bdc (022bdc) once per frame while driving. */
void BusDriveApplyGround_8c023cba(void);

#endif // _023938_BUS_DRIVE_H
