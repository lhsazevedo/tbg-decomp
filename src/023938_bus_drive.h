#ifndef _023938_BUS_DRIVE_H
#define _023938_BUS_DRIVE_H

typedef struct {
    float len_0x00;  /* remaining distance to the next point */
    float x_0x04;    /* next point's world x/z */
    float z_0x08;
    float dx_0x0c;   /* unit direction of the segment leading to it */
    float dz_0x10;
} LinePoint;

typedef struct {
    LinePoint *points_0x00;
    float length_0x04;
} LineBusSegment;

typedef struct {
    Uint16 leftLane_0x00;
    Uint16 rightLane_0x02;
    /* indexed by signalSide_0x25c */
    Uint16 altNext_0x04[3];
    Uint16 fallbackNext_0x0a;
} LineBusNode;

#define LINE_NODE_NONE 0xffff

/* Also idles the engine, and drops out of gear unless in reverse. */
void BusDriveEnterKnockback_8c023bce(void);

/* Also reseeds the position history and heading. */
void BusDriveSampleGround_8c023938(void);

/* Lane change: finds the point on the neighbouring lane's line to steer for. */
void BusDriveFindLaneTarget_8c023e7e(void);

/* Hitting a wall sets the knockback direction; also sets the bus's height. */
void BusDriveApplyGround_8c023cba(void);

#endif // _023938_BUS_DRIVE_H
