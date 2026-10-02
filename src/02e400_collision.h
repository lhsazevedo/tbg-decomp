#ifndef _02E400_COLLISION_H
#define _02E400_COLLISION_H

#include "014a9c_tasks.h"
#include "026710_traffic.h" /* TrafficEntry */

/* =======================
 * Non-initialized Globals
 * =======================
 */

extern Task *var_collisionScanCursor_8c228974;
extern NJS_BOX var_collisionSelfBox_8c228978;
extern NJS_BOX var_collisionCandidateBox_8c2289d8;

/* ====================
 * Functions
 * ====================
 */

/* Leaves entry's world-space box in var_collisionSelfBox_8c228978. */
TrafficEntry *CollisionFindTaskHit_8c02e400(
    /* The caller's own task, excluded from the scan */
    Task *self,
    TrafficEntry *entry
);

void CollisionQueueReset_8c02e486(void);

void CollisionQueueAdd_8c02e48e(void *obj);

/* Tests queued bounding spheres against whatever box the last
 * CollisionFindTaskHit_8c02e400 or BusCollisionFindHit_8c02e2dc left in
 * var_collisionSelfBox_8c228978. */
void *CollisionQueueTest_8c02e4ac(void);

#endif // _02E400_COLLISION_H
